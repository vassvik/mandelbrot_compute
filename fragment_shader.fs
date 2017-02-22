#version 430 core

in vec2 uv;

uniform sampler2D srcTex;

out vec3 color;

void main()
{
    color = texture(srcTex, uv).xyz;;
}