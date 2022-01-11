#version 430

layout(local_size_x = 256) in;


struct Particle
{
    mat4 Transform;

    vec2 Trajectory;
};


layout(std430, binding = 0) readonly buffer InParticlesBuffer
{

    Particle InParticles[];
};

layout(std430, binding = 1) writeonly buffer OutTransformsBuffer
{
    mat4 OutParticleTransforms[];
};



uniform mat4 ParticleEmmiterTransform;

uniform uint WindowWidth;
uniform uint WindowHeight;

uniform float ParticleScaleFactor;



vec2 CartesianToNDC(vec2 cartesianPosition)
{
    return vec2(((2.0f * cartesianPosition.x) / WindowWidth), 
                ((2.0f * cartesianPosition.y) / WindowHeight));
};


mat4 Translate(mat4 inputMatrix, vec3 translationVector)
{
    mat4 result = mat4(inputMatrix);

	result[3] = inputMatrix[0] * translationVector[0] + inputMatrix[1] * translationVector[1] + inputMatrix[2] * translationVector[2] + inputMatrix[3];

	return result;
};



void main()
{
    const Particle particle = InParticles[gl_GlobalInvocationID.x];

    const vec2 ndcPosition = CartesianToNDC(particle.Trajectory) / ParticleScaleFactor;
    
    const mat4 screenTransfrom = Translate(ParticleEmmiterTransform, vec3(ndcPosition, 0.0f))
                * particle.Transform;

    OutParticleTransforms[gl_GlobalInvocationID.x] = screenTransfrom;
};