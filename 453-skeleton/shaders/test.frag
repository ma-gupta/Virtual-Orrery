#version 330 core

in vec3 fragPos;
in vec2 tc;
in vec3 n;

uniform vec3 lightPos;
uniform sampler2D sampler;
uniform int shade;

out vec4 color;

void main() {
	//ambient
	float ambientStr = 0.05;

	//diffused
	vec3 lightDir = normalize(lightPos - fragPos);
    vec3 normal = normalize(n);
    float diff = max(dot(lightDir, normal), 0.0);

	//specular
	float specularStr = 0.7;
	vec3 viewDir = normalize(-fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	float specular = specularStr * spec;

	vec4 d = texture(sampler, tc);

	if (shade == 0){
		color = d;
	}
	else {
		// make sure specular lighting is only on the front of the sphere
		if (diff > 0.0f){
			color = (diff + ambientStr + specular) * d;
		}
		else{
			color = (diff + ambientStr) * d;
		}
	}
	
}
