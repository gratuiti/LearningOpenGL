#version 460 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D diffuseMap;
//uniform sampler2D normalMap;

//uniform bool normalMapping;

void main()
{           
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 viewDir = normalize(viewPos-fs_in.FragPos);
    vec3 halfwarDir = normalize(lightDir+viewDir);

     vec3 color = texture(diffuseMap,fs_in.TexCoords).rgb;
     vec3 ambient = 0.1 * color;
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float spec = pow(max(dot(normal,halfwarDir), 0.0), 32.0f);

    vec3 specular = vec3(0.2) * spec;

    //FragColor = vec4(color,1.0f);
    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}


