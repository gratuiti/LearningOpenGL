#version 460 core
out vec4 FragColor;


in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform sampler2D bloomBlur;
uniform float exposure;

uniform bool bloom;

void main(){
	const float gamma=2.2;
	vec3 hdrColor=texture(hdrBuffer,TexCoords).rgb;
	vec3 bloomColor=texture(bloomBlur,TexCoords).rgb;

	if(bloom)
		hdrColor += bloomColor; 
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	result=pow(result,vec3(1.0/gamma));
	FragColor=vec4(result,1.0);



}