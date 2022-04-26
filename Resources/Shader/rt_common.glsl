
#define INSTANCE_COUNT 1

struct InstanceDescriptor
{
	mat4 modelMat;

	uint64_t vertices; // 64 bit

	uint64_t indices; // 64 bit

	uint64_t materials; // 64 bit

    uint64_t placeholder;
};

struct Vertex
{
    vec3 pos;
    vec3 color;
    vec2 uvCoord;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    uint matIdx;
};

struct hitPayload
{
    vec3 hitValue;
    int recursion;
    //uint rngState;
};

// see https://github.com/nvpro-samples/vk_mini_path_tracer/blob/main/vk_mini_path_tracer/shaders/raytrace.comp.glsl
// Steps the RNG and returns a floating-point value between 0 and 1 inclusive.
float stepAndOutputRNGFloat(inout uint rngState)
{
  // Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
  rngState  = rngState * 747796405 + 1;
  uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
  word      = (word >> 22) ^ word;
  return float(word) / 4294967295.0f;
}

// see https://github.com/nvpro-samples/vk_mini_path_tracer/blob/main/vk_mini_path_tracer/shaders/raytrace.comp.glsl
vec3 randomDirection(inout uint rngState, vec3 centerVector, float offset)
{
    const float theta   = 6.2831853 * stepAndOutputRNGFloat(rngState);  // Random in [0, 2pi]
    const float u       = 2.0 * stepAndOutputRNGFloat(rngState) - 1.0;  // Random in [-1, 1]
    const float r       = sqrt(1.0 - u * u);
    vec3 direction      = centerVector + offset*vec3(r * cos(theta), r * sin(theta), u);
    return normalize(direction);
}
