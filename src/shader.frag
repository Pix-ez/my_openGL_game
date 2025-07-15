#version 420 core

in vec2 TexCoord;
in vec3 ourColor;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
   // Normalize vertex position to [0, 1] range for color
   //vec3 color = vertexPosition * 0.5 + 0.5;
   //FragColor = vec4(color, 1.0);
   // FragColor = ourColor;
   //FragColor = vec4(ourColor,1.0);

   //using texture co ordinate
   // FragColor = texture(texture2, TexCoord);
   vec4 base = texture(texture1, TexCoord);
   vec4 overlay = texture(texture2, TexCoord);
   FragColor = mix(overlay, base, overlay.b);
   // FragColor = overlay * overlay.a + base * (1.0 - overlay.a);



   // FragColor = mix(texture(texture1, TexCoord),texture(texture2, TexCoord), 1.0);
}