#version 430



layout(local_size_x = 256) in;


layout(std430, binding = 0) readonly buffer InTransformsBuffer
{
    float InValues[];
};

layout(std430, binding = 1) writeonly buffer OutTransformsBuffer
{
    float OutValues[];
};


uniform float Value;



void main()
{
    OutValues[gl_GlobalInvocationID.x] = InValues[gl_GlobalInvocationID.x] + Value;
};


