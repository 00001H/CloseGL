#include<bits/stdc++.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/ext/matrix_transform.hpp>

#define CGL_BINDLESS_SUPPORT
#define CGL_USE_DSA

#include"utils.hpp"
#include"glutils.hpp"
#include"cam.hpp"

#define window GLFWwindow*
#define kydown(ky) (glfwGetKey(win,ky)==GLFW_PRESS)

#define uint GLuint
#define uint64 GLuint64
const int sff = sizeof(float);
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::dvec2;
using glm::vec3;
using glm::perspective;

int SW,SH;
void inline onresize(window win,int w,int h){
    SW = w;
    SH = h;
    glViewport(0,0,w,h);
}

dvec2 getmousepos(window win){
    double x,y;
    glfwGetCursorPos(win,&x,&y);
    return dvec2(x,y);
}

int main(){
    SW=800;
    SH=600;
    initGLFW(4,6,GLFW_OPENGL_CORE_PROFILE);
    atexit(glfwTerminate);
    window win = mkwin(SW,SH,"CloseGL test");
    if(win==NULL){
        cout << "Window creation failed!" << endl;
        exit(-1);
    }
    glfwMakeContextCurrent(win);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        cout << "GLAD loading failed!" << endl;
        exit(-1);
    }
    glfwSetFramebufferSizeCallback(win,onresize);

    try{
        Shader shdr = Shader(loadStringFile("vertex.glsl"),loadStringFile("fragment.glsl"));
    }catch(shader_compilation_error &e){
        cout << e.getwhat() << endl;
    }
    float vdata[] = {
        -1.0,-1.0,0.0,
        -1.0,1.0,0.0,
        1.0,1.0,0.0,
        1.0,1.0,0.0,
        1.0,-1.0,0.0,
        -1.0,-1.0,0.0
    };

    while(!glfwWindowShouldClose(win)){
        glfwPollEvents();
        glClearColor(kydown(GLFW_KEY_ESCAPE)*0.8+0.2,0.3,0.3,1.0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(win);
    }
    return 0;
}
