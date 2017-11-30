#version 330

in vec4 position;
out vec2 vfUV;

void main()
{
    vfUV = position.xy;
    gl_Position = position;
}