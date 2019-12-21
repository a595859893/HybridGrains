#version 400
layout(location=0) in vec2 in_Position;
layout(location=1) in vec3 in_Color;
out vec3 fragmentColor;

void main(void){
	fragmentColor = in_Color;
	// gl_TexCoord[0] = gl_MultiTxCoord0;
	gl_Position = vec4(in_Position,0,1);
	gl_PointSize = 4;
}