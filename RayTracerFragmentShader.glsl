#version 430 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gPosition;
layout(location = 3) out int gID;
in vec2 TexCoords;

struct Camera {
	vec3 camPos;
	vec3 front;
	vec3 right;
	vec3 up;
	float halfH;
	float halfW;
	vec3 leftbottom;
	int LoopNum;
	mat4 SToW;
	mat4 WToS;
};

struct hitRecord {
	bool specular;
	float hitMin;
	int hitType;
	int bufferIndex;
	vec3 Normal;
	vec3 Pos;
	vec3 albedo;
	int materialIndex;
	vec3 dpdu;
	vec3 dpdv;
};

struct Ray {
	vec3 origin;
	vec3 direction;
	hitRecord rec;
};

struct Bound3f {
	vec3 pMin, pMax;
};

struct LinearBVHNode {
	vec3 pMin, pMax;
	int nPrimitives;
	int axis;
	int childOffset; //第二个子节点位置索引 或 基元起始位置索引
};

struct Sphere {
	vec3 center;
	float radius;
	vec3 albedo;
	int materialIndex;
};

struct Triangle {
	vec3 v0, v1, v2;
	vec3 n0, n1, n2;
	vec2 u0, u1, u2;
};
struct Lights
{
	int lightNum;
	int index[64];
	float data[64 * 13];
};

layout(std430, binding = 0) buffer lightsdata
{
	Lights lights;
};

uniform int primitiveDataSize[2];
uniform int materialDataSize[8];
uniform int screenWidth;
uniform int screenHeight;
uniform struct Camera camera;
uniform float randOrigin;
const float eps = 0.0001;
const float Pi = 3.14159265358979323846;
const float InvPi = 0.31830988618379067154;
const float Inv2Pi = 0.15915494309189533577;
const float Inv4Pi = 0.07957747154594766788;
const float PiOver2 = 1.57079632679489661923;
const float PiOver4 = 0.78539816339744830961;
const float Sqrt2 = 1.41421356237309504880;
const float ShadowEpsilon = 0.0001f;
uint wseed;
float rand(void);

uniform sampler2D texMesh;
uniform sampler2D texBvhNode;

vec3 shading(inout Ray r);
bool IntersectBound(Bound3f bounds, Ray ray, vec3 invDir, bool dirIsNeg[3]);

// 采样历史帧的纹理采样器
//uniform sampler2D historyTexture;

void main() {
	wseed = uint(randOrigin * float(6.95857) * (TexCoords.x * TexCoords.y));

	// 获取历史帧信息
	//vec3 hist = texture(historyTexture, TexCoords).rgb;

	vec2 tex = vec2(TexCoords.x, TexCoords.y);
	//tex.x = TexCoords.x + rand() / screenWidth;
	//tex.y = TexCoords.y + rand() / screenHeight;
	Ray cameraRay;
	cameraRay.origin = camera.camPos;
	//cameraRay.direction = normalize(camera.leftbottom + (tex.x * 2.0 * camera.halfW) * camera.right + (tex.y * 2.0 * camera.halfH) * camera.up);
	vec4 tmp = camera.SToW * vec4(tex.x, tex.y, 0, 1);
	tmp /= tmp.w;
	cameraRay.direction = normalize(vec3(tmp) - cameraRay.origin);
	//FragColor = vec4(cameraRay.direction, 1.0);
	//return;

	cameraRay.rec.hitMin = 100000.0;
	cameraRay.rec.bufferIndex = -1;

	vec3 curColor = shading(cameraRay);

	//curColor = (1.0 / float(camera.LoopNum)) * curColor + (float(camera.LoopNum - 1) / float(camera.LoopNum)) * hist;
	FragColor = vec4(curColor, 1.0);

}

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

float At(sampler2D dataTex, float index) {
	float row = (index + 0.5) / textureSize(dataTex, 0).x;
	float y = (int(row) + 0.5) / textureSize(dataTex, 0).y;
	float x = (index + 0.5 - int(row) * textureSize(dataTex, 0).x) / textureSize(dataTex, 0).x;
	vec2 texCoord = vec2(x, y);
	return texture2D(dataTex, texCoord).x;
}

LinearBVHNode getLinearBVHNode(int offset) {
	int offset1 = offset * (9);
	LinearBVHNode node;
	node.pMin = vec3(At(texBvhNode, float(offset1 + 0)), At(texBvhNode, float(offset1 + 1)), At(texBvhNode, float(offset1 + 2)));
	node.pMax = vec3(At(texBvhNode, float(offset1 + 3)), At(texBvhNode, float(offset1 + 4)), At(texBvhNode, float(offset1 + 5)));
	node.nPrimitives = int(At(texBvhNode, float(offset1 + 6)));
	node.axis = int(At(texBvhNode, float(offset1 + 7)));
	node.childOffset = int(At(texBvhNode, float(offset1 + 8)));
	return node;
}

Triangle getTriangle(int index) {
	Triangle tri_t;
	tri_t.v0.x = At(texMesh, float(index));
	tri_t.v0.y = At(texMesh, float(index + 1));
	tri_t.v0.z = At(texMesh, float(index + 2));

	tri_t.v1.x = At(texMesh, float(index + 3));
	tri_t.v1.y = At(texMesh, float(index + 4));
	tri_t.v1.z = At(texMesh, float(index + 5));

	tri_t.v2.x = At(texMesh, float(index + 6));
	tri_t.v2.y = At(texMesh, float(index + 7));
	tri_t.v2.z = At(texMesh, float(index + 8));

	return tri_t;
}

Sphere getSphere(int index) {
	Sphere sph_t;
	sph_t.center.x = At(texMesh, float(index));
	sph_t.center.y = At(texMesh, float(index + 1));
	sph_t.center.z = At(texMesh, float(index + 2));
	sph_t.radius = At(texMesh, float(index + 3));
	return sph_t;
}

void getMaterial(inout Ray ray)
{
	int offset = primitiveDataSize[ray.rec.hitType];
	ray.rec.materialIndex = int(At(texMesh, float(ray.rec.bufferIndex + offset)));
	if (ray.rec.materialIndex == 2)
		ray.rec.specular = true;
	else
		ray.rec.specular = false;
	ray.rec.albedo.x = At(texMesh, float(ray.rec.bufferIndex + offset + 1));
	ray.rec.albedo.y = At(texMesh, float(ray.rec.bufferIndex + offset + 2));
	ray.rec.albedo.z = At(texMesh, float(ray.rec.bufferIndex + offset + 3));
	return;
}

vec3 getTriangleNormal(Triangle tri, Ray ray) {
	vec3 normal = normalize(cross(tri.v2 - tri.v0, tri.v1 - tri.v0));
	//vec3 normal = normalize(cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
	if (dot(normal, ray.direction) > 0)
		normal = -normal;
	return normal;
}

void CoordinateSystem(vec3 v1, inout vec3 v2, inout vec3 v3)
{
	if (abs(v1.x) > abs(v1.y))
		v2 = vec3(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
	else
		v2 = vec3(0, v1.z, -v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
	v2 = normalize(v2);
	v3 = cross(v1, v2);
}

bool hitSphere(Sphere s, inout Ray r, int index) {
	vec3 oc = r.origin - s.center;
	float a = dot(r.direction, r.direction);
	float b = 2.0 * dot(oc, r.direction);
	float c = dot(oc, oc) - s.radius * s.radius;
	float discriminant = b * b - 4 * a * c;
	float dis;
	if (discriminant > 0.0) {
		dis = (-b - sqrt(discriminant)) / (2.0 * a);
		//if (dis > 0.0)
		//	dis = dis - 0.00001;
		//else return false;
		if (dis > 0.0)
			dis = dis;
		else return false;
	}
	else return false;

	bool hit = false;
	float dis_t = dis;
	if (dis_t > 0 && dis_t < r.rec.hitMin) {
		hit = true;
		r.rec.bufferIndex = index;
		r.rec.hitType = 1;
		r.rec.hitMin = dis_t;
		r.rec.Pos = r.origin + r.rec.hitMin * r.direction;
		r.rec.Normal = normalize(r.rec.Pos - s.center);
		CoordinateSystem(r.rec.Normal, r.rec.dpdu, r.rec.dpdv);
	}
	return hit;
}

// 返回值：ray到三角形交点的距离
bool hitTriangle(Triangle tri, inout Ray r, int index) {
	bool hit = false;
	// 找到三角形所在平面法向量
	vec3 A = tri.v1 - tri.v0;
	vec3 B = tri.v2 - tri.v0;
	vec3 N = normalize(cross(A, B));
	// Ray与平面平行，没有交点
	if (dot(N, r.direction) == 0) return hit;
	float D = -dot(N, tri.v0);
	float t = -(dot(N, r.origin) + D) / dot(N, r.direction);
	if (t < 0) return hit;
	// 计算交点
	vec3 pHit = r.origin + t * r.direction;
	vec3 edge0 = tri.v1 - tri.v0;
	vec3 C0 = pHit - tri.v0;
	//float eps = 0.00001;
	if (dot(N, cross(edge0, C0))+ eps < 0) return hit;
	vec3 edge1 = tri.v2 - tri.v1;
	vec3 C1 = pHit - tri.v1;
	if (dot(N, cross(edge1, C1))+ eps < 0) return hit;
	vec3 edge2 = tri.v0 - tri.v2;
	vec3 C2 = pHit - tri.v2;
	if (dot(N, cross(edge2, C2))+ eps < 0) return hit;

	//float dis_t = t - 0.000001;
	float dis_t = t;
	if (dis_t > 0 && dis_t < r.rec.hitMin) {
		hit = true;
		r.rec.bufferIndex = index;
		r.rec.hitType = 0;
		r.rec.hitMin = dis_t;
		r.rec.Pos = r.origin + r.rec.hitMin * r.direction;
		r.rec.Normal = getTriangleNormal(tri, r);
		CoordinateSystem(r.rec.Normal, r.rec.dpdu, r.rec.dpdv);
	}
	return hit;
}

bool hitPrimitive(inout Ray ray, inout int global_offset)
{
	bool hit = false;
	int hitType = int(round(At(texMesh, float(global_offset))));
	int matType = int(round(At(texMesh, float(global_offset + primitiveDataSize[hitType]))));
	if (hitType == 0)
	{
		Triangle tri_t = getTriangle(global_offset + 1);
		hit = hitTriangle(tri_t, ray, global_offset);
		global_offset += primitiveDataSize[hitType] + materialDataSize[matType];
		return hit;
	}
	else
	{
		Sphere sph_t = getSphere(global_offset + 1);
		hit = hitSphere(sph_t, ray, global_offset);
		global_offset += primitiveDataSize[hitType] + materialDataSize[matType];
		return hit;
	}
}

bool IntersectBVH(inout Ray ray) {
	// if (!bvhTree.nodes) return false;
	bool hit = false;

	vec3 invDir = vec3(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
	bool dirIsNeg[3];
	dirIsNeg[0] = (invDir.x < 0.0); dirIsNeg[1] = (invDir.y < 0.0); dirIsNeg[2] = (invDir.z < 0.0);
	// Follow ray through BVH nodes to find primitive intersections
	int toVisitOffset = 0, currentNodeIndex = 0;
	int nodesToVisit[64];

	while (true) {
		LinearBVHNode node = getLinearBVHNode(currentNodeIndex);
		// Ray 与 BVH的交点
		Bound3f bound; bound.pMin = node.pMin; bound.pMax = node.pMax;
		if (IntersectBound(bound, ray, invDir, dirIsNeg)) {
			if (node.nPrimitives > 0) {
				// Ray 与 叶节点的交点
				int global_offset = node.childOffset;
				for (int i = 0; i < node.nPrimitives; ++i) {
					if(hitPrimitive(ray, global_offset))
						hit = true;
				}
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
			else {
				// 把 BVH node 放入 _nodesToVisit_ stack, advance to near
				if (bool(dirIsNeg[node.axis])) {
					nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
					currentNodeIndex = node.childOffset;
				}
				else {
					nodesToVisit[toVisitOffset++] = node.childOffset;
					currentNodeIndex = currentNodeIndex + 1;
				}
			}
		}
		else {
			if (toVisitOffset == 0) break;
			currentNodeIndex = nodesToVisit[--toVisitOffset];
		}
	}
	return hit;
}

vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0 * vec3(rand(), rand(), rand()) - vec3(1, 1, 1);
	} while (dot(p, p) >= 1.0);
	return p;
}

vec3 diffuseReflection(vec3 Normal) {
	return normalize(Normal + random_in_unit_sphere());
}

vec3 metalReflection(vec3 rayIn, vec3 Normal) {
	return normalize(rayIn - 2 * dot(rayIn, Normal) * Normal + 0.15 * random_in_unit_sphere());
}

vec2 UniformSampleTriangle() {
	float su0 = sqrt(rand());
	return vec2(1 - su0, rand() * su0);
}

vec3 OffsetRayOrigin(vec3 p, vec3 n, vec3 wi)
{
	float d = dot(abs(n), vec3(eps, eps, eps));
	vec3 offset = d * n;
	float tmp = dot(wi, n) < 0 ? -1: 1;
	vec3 po = p + tmp * offset;
	return po;
}

vec3 WorldToLocal(vec3 v, Ray ray)
{
	return vec3(dot(v, ray.rec.dpdu), dot(v, ray.rec.dpdv), dot(v, ray.rec.Normal));
}

vec3 LocalToWorld(vec3 v, Ray ray) 
{
	return vec3(ray.rec.dpdu.x * v.x + ray.rec.dpdv.x * v.y + ray.rec.Normal.x * v.z,
		ray.rec.dpdu.y * v.x + ray.rec.dpdv.y * v.y + ray.rec.Normal.y * v.z,
		ray.rec.dpdu.z * v.x + ray.rec.dpdv.z * v.y + ray.rec.Normal.z * v.z);
}

vec3 getBSDF(vec3 wo, vec3 wi, Ray r)
{
	if (r.rec.materialIndex == 0)
	{
		return r.rec.albedo * InvPi;
	}
	else
	{
		return vec3(0, 0, 0);
	}
}

float getPdf(vec3 wo, vec3 wi, Ray r)
{
	if (r.rec.materialIndex == 0)
	{
		return wo.z * wi.z > 0 ? abs(wi.z) * InvPi : 0;
	}
	else
	{
		return 0;
	}
}


bool Sample_Li(int randlight, Ray r, inout float pdf, inout vec3 f,inout Ray vis)
{
	int index = lights.index[randlight];
	int lightType = int(round(lights.data[index]));
	index++;
	if (lightType == 0)
	{
		Triangle tri;
		vec3 albedo;
		for (int i = 0; i < 3; ++i)
			tri.v0[i] = lights.data[index + i];
		index += 3;
		for (int i = 0; i < 3; ++i)
			tri.v1[i] = lights.data[index + i];
		index += 3;
		for (int i = 0; i < 3; ++i)
			tri.v2[i] = lights.data[index + i];
		index += 3;
		for (int i = 0; i < 3; ++i)
			albedo[i] = lights.data[index + i];
		index += 3;
		vec2 b = UniformSampleTriangle();
		vec3 pos = b[0] * tri.v0 + b[1] * tri.v1 + (1 - b[0] - b[1]) * tri.v2;
		vec3 normal = cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
		pdf = 2 / length(normal);
		normal = normalize(normal);

		vec3 wi = pos - r.rec.Pos;
		float dist = length(wi);
		if (dist < eps)
			return false;

		wi = normalize(wi);

		float dot1 = dot(r.rec.Normal, wi);
		float dot2 = dot(normal, wi);
		if (abs(dot1) < eps || abs(dot2) < eps || dot1 < 0)
			return false;
		//if (dot2 < 0)
		//	normal = -normal;
		if (dot2 < 0)
			return false;

		pdf *= dist * dist / abs(dot2);


		vec3 wiLocal = WorldToLocal(wi, r), woLocal = WorldToLocal(-r.direction, r);
		f = getBSDF(woLocal, wiLocal, r);
		if (length(f) < eps)
			return false;
		f *= albedo * abs(dot(wi, r.rec.Normal));

		//Ray vis;
		vis.origin = OffsetRayOrigin(r.rec.Pos, r.rec.Normal, wi);
		vec3 target = OffsetRayOrigin(pos, normal, -wi);
		vis.direction = target - vis.origin;
		vis.rec.hitMin = 1 - eps;
		//vec3 target = OffsetRayOrigin(r.rec.Pos, r.rec.Normal, wi);
		//vis.origin = OffsetRayOrigin(pos, normal, -wi);
		//vis.direction = target - vis.origin;
		//vis.rec.hitMin = 1 - eps;
		bool visibility = !IntersectBVH(vis);
		return visibility;
	}
	else
	{
		return false;
	}
}

vec2 ConcentricSampleDisk() {
	vec2 uOffset = 2.f * vec2(rand(),rand()) - vec2(1, 1);
	//if (uOffset.x == 0 && uOffset.y == 0) 
	//	return vec2(0, 0);
	float theta, r;
	if (abs(uOffset.x) > abs(uOffset.y)) 
	{
		r = uOffset.x;
		theta = PiOver4 * (uOffset.y / uOffset.x);
	}
	else 
	{
		r = uOffset.y;
		theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
	}
	return r * vec2(cos(theta), sin(theta));
}

vec3 CosineSampleHemisphere() 
{
	vec2 d = ConcentricSampleDisk();
	float z = sqrt(max(0.f, 1 - d.x * d.x - d.y * d.y));
	return vec3(d.x, d.y, z);
}

vec3 Sample_f(Ray ray, vec3 wo, inout vec3 wi, inout float pdf)
{
	vec3 wiLocal, woLocal = WorldToLocal(wo,ray);
	if (ray.rec.materialIndex == 0)
	{
		wiLocal = CosineSampleHemisphere();
		if (woLocal.z < 0) 
			wiLocal.z *= -1;
		//wiLocal = normalize(wiLocal);
		pdf = getPdf(woLocal, wiLocal, ray);
		vec3 f = getBSDF(woLocal, wiLocal, ray);
		wi = LocalToWorld(wiLocal, ray);
		return f;
	}
	else if(ray.rec.materialIndex == 2)
	{
		wiLocal = normalize(vec3(-woLocal.x, -woLocal.y, woLocal.z));
		pdf = 1;
		wi = LocalToWorld(wiLocal, ray);
		return ray.rec.albedo / abs(wiLocal.z);
	}
	return vec3(0, 0, 0);
}

vec3 shading(inout Ray r) 
{
	vec3 L = vec3(0.f, 0.f, 0.f);
	vec3 beta = vec3(1.f, 1.f, 1.f);
	bool specularBounce = false;
	float rrThreshold = 1.f;
	for (int bounces = 0; bounces < 50; ++bounces)
	{
		bool foundIntersection = IntersectBVH(r);
		if(foundIntersection)
			getMaterial(r);
		if (bounces == 0)
		{
			gNormal = r.rec.Normal;
			gPosition = r.rec.Pos;
			gID = r.rec.bufferIndex;
		}
		if (bounces == 0 || specularBounce) {
			// Add emitted light at path vertex or from the environment
			if (foundIntersection) {
				if (r.rec.materialIndex == 1)
					L += beta * r.rec.albedo;
			}
			else {
				// Add infinite area lights
				//float t = 0.5 * (r.direction.y + 1.0);
				//L += (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
			}
		}

		if (!foundIntersection)
			break;

		//Sample illumination from lights to find path contribution.
		//(But skip this for perfectly specular BSDFs.)
		if (!r.rec.specular && lights.lightNum > 0)
		{
			int randlight= min(int(rand() * lights.lightNum), lights.lightNum - 1);
			float lightPdf = 1.f / lights.lightNum;
			float pdf = 0;
			vec3 f;
			Ray vis;
			bool visibility = Sample_Li(randlight, r, pdf, f, vis);
			if (visibility)
			{
				//if (bounces == 1)
				//{
				//	return sqrt(beta * f / pdf / lightPdf);
				//}
				L += beta * f / pdf / lightPdf;
				//return vec3(0, 0, 0);
			}
			//else
			//{
				//return vis.rec.Pos / 10.f;
				//return vec3(0, 0, 0);
			//}
		}


		//Sample BSDF to get new path direction
		vec3 wo = -r.direction, wi;
		float pdf;
		vec3 f = Sample_f(r, wo, wi, pdf);
		if (length(f) < eps || pdf < eps)
			break;
		wi = normalize(wi);
		beta *= f * abs(dot(wi, r.rec.Normal)) / pdf;
		//beta *= f;
		specularBounce = r.rec.specular;

		vec3 origin = OffsetRayOrigin(r.rec.Pos, r.rec.Normal, wi);
		r.origin = origin;
		r.direction = wi;
		r.rec.hitMin = 100000.0;
		r.rec.specular = false;

		vec3 rrBeta = beta;
		float MaxComponentValue = max(rrBeta.z, max(rrBeta.x, rrBeta.y));
		if (MaxComponentValue < rrThreshold && bounces > 3)
		{
			float q = max(0.05f, 1 - MaxComponentValue);
			if (rand() < q) 
				break;
			//beta /= 1 - q;
		}
	}
	L = sqrt(L);
	return L;
}

vec3 getBoundp(Bound3f bound, int i) {
	return (i == 0) ? bound.pMin : bound.pMax;
}
bool IntersectBound(Bound3f bounds, Ray ray, vec3 invDir, bool dirIsNeg[3]) {
	// Check for ray intersection against $x$ and $y$ slabs
	float tMin = (getBoundp(bounds, int(dirIsNeg[0])).x - ray.origin.x) * invDir.x;
	float tMax = (getBoundp(bounds, 1 - int(dirIsNeg[0])).x - ray.origin.x) * invDir.x;
	float tyMin = (getBoundp(bounds, int(dirIsNeg[1])).y - ray.origin.y) * invDir.y;
	float tyMax = (getBoundp(bounds, 1 - int(dirIsNeg[1])).y - ray.origin.y) * invDir.y;

	// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
	if (tMin > tyMax || tyMin > tMax) return false;
	if (tyMin > tMin) tMin = tyMin;
	if (tyMax < tMax) tMax = tyMax;

	// Check for ray intersection against $z$ slab
	float tzMin = (getBoundp(bounds, int(dirIsNeg[2])).z - ray.origin.z) * invDir.z;
	float tzMax = (getBoundp(bounds, 1 - int(dirIsNeg[2])).z - ray.origin.z) * invDir.z;

	// Update _tzMax_ to ensure robust bounds intersection
	if (tMin > tzMax || tzMin > tMax) return false;
	if (tzMin > tMin) tMin = tzMin;
	if (tzMax < tMax) tMax = tzMax;

	return tMax > 0;
}









