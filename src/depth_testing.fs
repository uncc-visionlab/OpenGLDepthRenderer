// FRAGMENT SHADER
#version 330 core
out vec4 FragColor;

//float near = 0.1; 
//float far = 100.0; 
uniform float near = 0.1; 
uniform float far = 100.0; 

float LinearizeDepthInf(float depth) 
{
    float z = - depth  - 1.0; // back to NDC 
    return (2.0 * near) / (1 - z );	
}

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    //float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    float depth = LinearizeDepthInf(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    FragColor = vec4(vec3(1.0 - depth), 1.0);
}