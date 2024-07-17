#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int screenWidth;
uniform int screenHeight;

uniform sampler2D screenTexture;

void main() {
	vec3 col = texture(screenTexture, TexCoords).rgb;
	FragColor = vec4(col, 1.0);
    return;
}

