
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



uniform mat4 ParticleTransform;
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


float RandomNumberGenerator(vec2 uv, float seed)
{
    float fixedSeed = abs(seed) + 1.0;

    float x = dot(uv, vec2(12.9898, 78.233) * fixedSeed);

    return fract(sin(x) * 43758.5453);
};


float RandomNumberGenerator(vec2 uv, float seed, float min, float max)
{
    const float rng = RandomNumberGenerator(uv, seed);
    
    // Map [0, 1] to [min, max] 
    const float rngResult = min + rng * (max - min);

    return rngResult;
};




void InitializeParticleValues(inout Particle particle)
{
    const float newTrajectoryA = RandomNumberGenerator(particle.Trajectory, 0.318309f, 0.01f, 0.1f);


    // A very simple way of creating some trajectory variation
    const float newTrajectoryB = (mod(gl_GlobalInvocationID.x, 2)) == 0 ?
        -RandomNumberGenerator(particle.Trajectory, 0.318309f, 4.4f, 4.5f) :
        RandomNumberGenerator(particle.Trajectory, 0.318309f, 4.4f, 4.5f);

    // const float newOpacityDecreaseRate = _opacityDecreaseRateDistribution(_rng.get());


    // Correct the rate depending on trajectory direction
    const float newRate = sign(newTrajectoryB) == -1.0f ?
        // "Left" trajectory 
        -RandomNumberGenerator(particle.Trajectory, 0.318309f, 0.25f, 0.5f) :
        // "Right" trajectory
        RandomNumberGenerator(particle.Trajectory, 0.318309f, 0.25f, 0.5f);



    particle.TrajectoryA = newTrajectoryA;
    particle.TrajectoryB = newTrajectoryB;

    particle.Trajectory = vec2(0.0f);
    particle.Rate = newRate;

    particle.Transform = ParticleTransform;
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
    
    mat4 screenTransfrom = (Translate(ParticleEmmiterTransform, vec3(ndcPosition.x, ndcPosition.y, 0.0f))) * particle.Transform;

    const vec3 screenPosition = vec3(screenTransfrom[3]);


    
    // If the particle is outside screen bounds..
    if (screenPosition.y < -1.0f)
    {
        // "Reset" the particle
        InitializeParticleValues(particle);
    };


    OutParticles[gl_GlobalInvocationID.x] = particle;
    OutParticleScreenTransforms[gl_GlobalInvocationID.x] = screenTransfrom;
};