#version 420 core
in vec3 vertexPosition;
//in vec3 ourColor;
uniform vec4 ourColor;
out vec4 FragColor;
void main()
{
   // Normalize vertex position to [0, 1] range for color
   //vec3 color = vertexPosition * 0.5 + 0.5;
   //FragColor = vec4(color, 1.0);
   FragColor = ourColor;
   //FragColor = vec4(ourColor,1.0);
}