#version 430 core

// Input verasdtex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 uvCoord;

out vec2 uv;

void main(){
	uv = uvCoord;
    gl_Position.xyz = vertexPosition;
    gl_Position.w = 1.0;
}

