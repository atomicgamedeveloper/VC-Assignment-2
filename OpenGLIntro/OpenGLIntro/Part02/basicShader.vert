#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

out vec2 texturecoords;


// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V; // view matrix
uniform mat4 M; // model matrix
uniform mat4 P; //projection matrix


void main(){

    gl_Position =  MVP* vec4(vertexPosition_modelspace,1);
    texturecoords = gl_Position.xy;
}

