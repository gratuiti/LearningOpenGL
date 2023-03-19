#version 460 core
layout (location=0) out vec4 FragColor;
layout (location=1) out vec4 BrightColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 lightcolor;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};



uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos-FragPos);

    //vec3 result = CalcDirLight(dirLight, norm, viewDir);
    vec3 result = CalcPointLight(pointLight, norm, FragPos, viewDir);    


    FragColor = vec4(result,1.0f);

    float brightness=dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness>1.0){
        BrightColor=vec4(FragColor.rgb,1.0f);
    }else{
        BrightColor=vec4(0.0,0.0,0.0,0.0);
    }

}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwarDir = normalize(lightDir+viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal,halfwarDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwarDir = normalize(lightDir+viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal,halfwarDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.lightcolor*light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.lightcolor*light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}