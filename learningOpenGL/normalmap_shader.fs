#version 460 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;


struct Material{
    sampler2D texture_diffuse;
    sampler2D texture_normal;
    sampler2D texture_specular;
    float shininess;
};


uniform Material material;



void main()
{           
    // Obtain normal from normal map in range [0,1]
    vec3 norm = texture(material.texture_normal, fs_in.TexCoords).rgb;
    // Transform normal vector to range [-1,1]
    norm = normalize(norm * 2.0 - 1.0);  // this normal is in tangent space

    // Get diffuse color
    vec3 color = texture(material.texture_diffuse, fs_in.TexCoords).rgb;
    // Ambient
    vec3 ambient = 0.1 * color;
    // Diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = diff * color;
    // Specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec * vec3(texture(material.texture_specular, fs_in.TexCoords));
    
    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}