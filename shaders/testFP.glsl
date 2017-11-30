// raymarch.frag
#version 330

in vec2 vfUV;
out vec4 FragColor;

void main()
{
    vec2 uv = vfUV;
	FragColor = vec4(uv,0,1); 
}