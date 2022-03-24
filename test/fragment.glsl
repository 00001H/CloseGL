#version 420 core

layout(location=0) in vec3 fragcolor;

out vec4 color;

void main(){
    color = vec4(fragcolor,1.0);
}