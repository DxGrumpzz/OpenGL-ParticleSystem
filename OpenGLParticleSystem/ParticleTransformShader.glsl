#version 430


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;



layout(std430, binding = 0) readonly buffer InTransformsBuffer
{
    float InValues[];
};

layout(std430, binding = 1) writeonly buffer OutTransformsBuffer
{
    float OutValues[];
};


void main()
{
    
    OutValues[gl_GlobalInvocationID.x] = InValues[gl_GlobalInvocationID.x];

};