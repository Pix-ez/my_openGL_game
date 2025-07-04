#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vertexPosition;  // Pass to fragment shader
//out vec3 ourColor;  // Pass to fragment shader
void main()
{
   vertexPosition = aPos;  // Pass through
   gl_Position = vec4(aPos, 1.0);
   //ourColor = aColor;
}

