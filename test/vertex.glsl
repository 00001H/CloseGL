#version 420 core

layout(location=0) in vec3 vpos;
layout(location=1) in vec3 vcolor;

layout(location=0) out vec3 ficolor;

void main(){
    gl_Position = vec4(vpos,1.0);
    ficolor = vcolor;
}