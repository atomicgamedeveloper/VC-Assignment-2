#version 330 core

// Ouput data
out vec3 color;

uniform vec4 colorValue;
void main()
{

	// Output color = red 
	color = colorValue.rgb;
   // color.g = 1.0;

}
