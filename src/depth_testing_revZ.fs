// FRAGMENT SHADER
#version 330 core
out vec4 FragColor;

//float near = 0.1; 
//float far = 100.0; 
uniform float near; 
uniform float far; 

float LinearizeRevDepthInf(float depth) {
    return near / depth;	
}

float LinearizeRevDepth(float depth) {
    return -(far * near) / (near + depth * (far - near));	
}

void main()
{             
    float depth = LinearizeRevDepthInf(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    FragColor = vec4(vec3(1.0 - depth/10), 0.0);
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}