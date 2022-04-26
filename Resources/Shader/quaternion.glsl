
#define PI 3.141592f

// see https://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/

vec4 quat(vec3 axis, float angle)
{
	float halfAngle = (angle/2.0)*(PI/180.0);
	return vec4(axis*sin(halfAngle), cos(halfAngle));
}

vec4 quat(vec3 angularVelocity)
{
	vec3 axis = normalize(angularVelocity);
	float angle = length(angularVelocity);
	float halfAngle = (angle/2.0)*(PI/180.0);
	return vec4(axis*sin(halfAngle), cos(halfAngle));
}

vec4 inv_quat(vec4 q)
{
	return vec4(-q.xyz,q.w);
}

vec4 mult_quat(vec4 q1, vec4 q2)
{
	vec4 qr;
	qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
	qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
	qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
	qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
	return qr;
}

// see https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

mat4 quat_get_matrix(vec4 q)
{
	mat4 A,B;
	A[0]	= vec4(q.w, q.z, -q.y, q.x);
	A[1]	= vec4(-q.z, q.w, q.x, q.y);
	A[2]	= vec4(q.y, -q.x, q.w, q.z);
	A[3]	= vec4(-q.x, -q.y, -q.z, q.w);

	B[0]	= vec4(q.w, q.z, -q.y, -q.x);
	B[1]	= vec4(-q.z, q.w, q.x, -q.y);
	B[2]	= vec4(q.y, -q.x, q.w, -q.z);
	B[3]	= vec4(q.x, q.y, q.z, q.w);
	return A*B;
}
