#version 430

layout(local_size_x = 256) in;


struct Particle
{
    float TrajectoryA;
    float TrajectoryB;

    vec2 Trajectory;
    
    mat4 Transform;
    
    float Rate;
};


layout(std430, binding = 0) readonly buffer InParticlesBuffer
{
    Particle InParticles[];
};

layout(std430, binding = 1) writeonly buffer OutTransformsBuffer
{
    Particle OutParticles[];
};

layout(std430, binding = 2) writeonly buffer OutParticleScreenTransformsBuffer
{
    mat4 OutParticleScreenTransforms[];
};



uniform mat4 ParticleEmmiterTransform;

uniform uint WindowWidth;
uniform uint WindowHeight;

uniform float ParticleScaleFactor;

uniform float DeltaTime;



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


float ParticleTrajectoryFunction(float particleX, float a = 1.0f, float b = 1.0f)
{
    return particleX * (((-a) * particleX) + b);
};



void main()
{
    Particle particle = InParticles[gl_GlobalInvocationID.x];


    // Calculate next trajectory position
    particle.Trajectory.x += particle.Rate * DeltaTime;
    particle.Trajectory.y = ParticleTrajectoryFunction(particle.Trajectory.x, particle.TrajectoryA, particle.TrajectoryB);

    // Update opacity
    // particle.Opacity -= particle.OpacityDecreaseRate * DeltaTime;


    const vec2 ndcPosition = CartesianToNDC(particle.Trajectory) / ParticleScaleFactor;
    
    const mat4 screenTransfrom = (Translate(ParticleEmmiterTransform, vec3(ndcPosition.x, ndcPosition.y, 0.0f))) * particle.Transform;


    OutParticles[gl_GlobalInvocationID.x] = particle;
    OutParticleScreenTransforms[gl_GlobalInvocationID.x] = screenTransfrom;

};