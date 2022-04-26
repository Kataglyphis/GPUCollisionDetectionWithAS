struct Particle {
	   
	vec4		position;
	vec4		color;
	vec4		velocity;
	vec4		acceleration;

};

struct ComputeUBO {
	int num_particles;
};

// Push constant structure for the compute
struct PushConstantCompute
{
	vec4			numberAndTypeOfParticles;	// in X,Y,Z,  W = particle Type 
	vec4			limitsAndTime;				// X,Y,Z = limits; w = delta_t
	vec4			velocities;					// X,Y,Z = velocity of vector fields;, W = velocity particles
	vec4			view;						// view vector
	mat4			particleModel;

};


struct DepthPeelingConstants {
	mat4 view;
	mat4 projection;
};

struct Node
{
	vec4	position;

	float	depth;
	uint	next;

	uint	placeholder1;
	uint	placeholder2;

};

struct Counter {

	uint	count;
	uint	maxNodeCount;
	uint	particle_count;
	uint	placeholder1;

};