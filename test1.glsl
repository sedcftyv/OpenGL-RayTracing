#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int screenWidth;
uniform int screenHeight;

struct Camera {
	vec3 camPos;
	vec3 front;
	vec3 right;
	vec3 up;
	float halfH;
	float halfW;
	vec3 leftbottom;
	int LoopNum;
};
uniform struct Camera camera;

uniform float randOrigin;
uint wseed;
float randcore(uint seed) {
	seed = (seed ^ uint(61)) ^ (seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> uint(4));
	seed *= uint(0x27d4eb2d);
	wseed = seed ^ (seed >> uint(15));
	return float(wseed) * (1.0 / 4294967296.0);
}
float rand() {
	return randcore(wseed);
}

// ������ʷ֡�����������
uniform sampler2D historyTexture;

void main() {
	wseed = uint(randOrigin * float(6.95857) * (TexCoords.x * TexCoords.y));
	//if (distance(TexCoords, vec2(0.5, 0.5)) < 0.4)
	//	FragColor = vec4(rand(), rand(), rand(), 1.0);
	//else
	//	FragColor = vec4(0.0, 0.0, 0.0, 1.0);

	// ��ȡ��ʷ֡��Ϣ
	vec3 hist = texture(historyTexture, TexCoords).rgb;

	vec3 rayDir = normalize(camera.leftbottom + (TexCoords.x * 2.0 * camera.halfW) * camera.right + (TexCoords.y * 2.0 * camera.halfH) * camera.up);

	float radius = 1.2;
	vec3 sphereCenter = vec3(0.0, 0.0, 0.0);
	vec3 oc = camera.camPos - sphereCenter;
	float a = dot(rayDir, rayDir);
	float b = 2.0 * dot(oc, rayDir);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;

	if (discriminant > 0.0) {
		float dis = (-b - sqrt(discriminant)) / (2.0 * a);
		if (dis > 0.0) {
			vec3 N = normalize(camera.camPos + dis * rayDir - sphereCenter);
			vec3 LightDir = normalize(vec3(1.0, 1.0, 4.0));
			float dif = max(dot(N, LightDir), 0.0);
			float spec = pow(max(dot(N, LightDir), 0.0), 64);
			float lu = 0.1 + 0.5 * dif + 0.4 * spec;
			//FragColor = vec4(lu, lu, lu, 1.0);

			vec3 curColor = vec3(rand(), rand(), rand()) ;
			FragColor = vec4(curColor, 1.0);
		}
		else {
			FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	else
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}





