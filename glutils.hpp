#ifndef CGL_GLUTILS
#define CGL_GLUTILS
#include<iostream>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<GLFW/glfw3.h>
#include<GL/gl.h>

//#define CGL_BINDLESS_SUPPORT
//#define CGL_USE_DSA


//Textures
struct TextureData{
    GLenum color_mode;//GL_RGB or GL_RGBA
    int color_channels;//int, must correspond to color_mode.
    int width;
    int height;
    unsigned char* data;//data pointer
    ~TextureData(){
        free_data();
    }
    void free_data(){//Note: automatically freed on destruct, will set data to nullptr
        if(data!=nullptr){
            stbi_image_free(data);
            data = nullptr;
        }
    }
};
void set_flip(bool flip=true){//Fixes texture flipping problem.
    stbi_set_flip_vertically_on_load(flip);
}
class texture_loading_error:public std::logic_error{
    using std::logic_error::logic_error;
};
TextureData loadTextureData(std::string filename,int desired_channels=0){
    TextureData tdata;
    unsigned char *data = stbi_load(filename.c_str(),&tdata.width,&tdata.height,&tdata.color_channels,desired_channels);
    if(!data){
        throw texture_loading_error("stbi_load failed");
    }
    switch(tdata.color_channels){
        case 3:{
            tdata.color_mode = GL_RGB;
            break;
        }
        case 4:{
            tdata.color_mode = GL_RGBA;
            break;
        }
        default:{
            tdata.color_mode = -1;
        }
    }
    return tdata;
}
void cofigureTexture2D(TextureData dat,GLenum buffer_color_mode=GL_RGBA,GLint mipmap_level=0,GLenum data_format=GL_UNSIGNED_BYTE){
    glTexImage2D(GL_TEXTURE_2D,mipmap_level,buffer_color_mode,dat.width,dat.height,0,dat.color_mode,GL_UNSIGNED_BYTE,dat.data);
}
typedef GLuint GLTexture;
#ifdef CGL_BINDLESS_SUPPORT
unsigned __int64 genBindlessTextureHandle(GLuint texture){
    unsigned __int64 hndl = glGetTextureHandleARB(texture);
    glMakeTextureHandleResidentARB(hndl);
    return hndl;
}
#endif
void inline activeTexture(GLint texunit){
    glActiveTexture(GL_TEXTURE0+texunit);
}
void inline bindTexture(GLuint texture,GLenum texture_type=GL_TEXTURE_2D){
    glBindTexture(texture_type,texture);
}
//GLFW
void initGLFW(int major,int minor,int profile){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE,profile);
}
GLFWwindow inline *mkwin(int width,int height,const char* title,GLFWmonitor* monitor=NULL,GLFWwindow* share=NULL){
    return glfwCreateWindow(width,height,title,monitor,share);
}
//Shaders
class shader_compilation_error:std::exception{
    private:
        std::string reason;
        const char *reason_cstr;
    public:
        shader_compilation_error(std::string reason){
            this->reason = reason;
            this->reason_cstr = reason.c_str();
        }
        std::string inline getwhat() const{
            return reason;
        }
        const char inline *what() const noexcept{
            return reason_cstr;
        }
};
class program_linking_error:public std::logic_error{
    using std::logic_error::logic_error;
};
GLuint loadvshad(std::string vssource){
    GLuint vshad = glCreateShader(GL_VERTEX_SHADER);
    {
        const char* cstrcode = vssource.c_str();
        glShaderSource(vshad,1,&cstrcode,NULL);
    }
    glCompileShader(vshad);
    int success;
    char errormsg[2048];
    glGetShaderiv(vshad,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(vshad,1024,NULL,errormsg);
        throw shader_compilation_error(errormsg);
    }
    return vshad;
}
GLuint loadfshad(std::string fssource){
    GLuint fshad = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* cstrcode = fssource.c_str();
        glShaderSource(fshad,1,&cstrcode,NULL);
    }
    glCompileShader(fshad);
    int success;
    char errormsg[2048];
    glGetShaderiv(fshad,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(fshad,1024,NULL,errormsg);
        throw shader_compilation_error(errormsg);
    }
    return fshad;
}
GLuint generateLinkedProgram(GLuint attachments[],size_t count){
    GLuint program = glCreateProgram();
    for(size_t i=0;i<count;i++){
        glAttachShader(program,attachments[i]);
    }
    glLinkProgram(program);
    GLint success;
    char errormsg[1024];
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success){
        glGetProgramInfoLog(program,1024,NULL,errormsg);
        throw program_linking_error(errormsg);
        return -1;
    }
    return program;
}
GLuint inline generateLinkedProgram(GLuint vshad,GLuint fshad){
    GLuint attachments[] = {vshad,fshad};
    return generateLinkedProgram(attachments,2);
}
class Shader{
    public:
        GLuint id;
        std::string vshadsource,fshadsource;
        Shader(){
            id = -1;
        }
        Shader(std::string vshadsource,std::string fshadsource,bool compile=true){
            modify(vshadsource,fshadsource,compile);
        }
        void modify(std::string newvshadsource,std::string newfshadsource,bool recompile=true){
            vshadsource = newvshadsource;
            fshadsource = newfshadsource;
            if(recompile)this->recompile();
        }
        void recompile(){
            GLuint vshadi = loadvshad(vshadsource);
            GLuint fshadi = loadfshad(fshadsource);
            id = generateLinkedProgram(vshadi,fshadi);
            glDeleteShader(vshadi);
            glDeleteShader(fshadi);
        }
        void use(){
            glUseProgram(id);
        }
        void uniform1i(const char* name,GLint x){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniform1i(id,location,x);
            #else
            glUseProgram(id);
            glUniform1i(location,x);
            #endif
        }
        void uniform1f(const char* name,GLfloat x){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniform1f(id,location,x);
            #else
            glUseProgram(id);
            glUniform1f(location,x);
            #endif
        }
        void uniform3f(const char* name,GLfloat x,GLfloat y,GLfloat z){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniform3f(id,location,x,y,z);
            #else
            glUseProgram(id);
            glUniform3f(location,x,y,z);
            #endif
        }
        void inline uniform3fv(const char* name,glm::vec3 vector){
            uniform3f(name,vector.x,vector.y,vector.z);
        }
        void uniform4f(const char* name,GLfloat x,GLfloat y,GLfloat z,GLfloat w){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniform4f(id,location,x,y,z,w);
            #else
            glUseProgram(id);
            glUniform4f(location,x,y,z,w);
            #endif
        }
        void inline uniform4fv(const char* name,glm::vec4 vector){
            uniform4f(name,vector.x,vector.y,vector.z,vector.w);
        }
        void uniformMatrix3fv(const char* name,glm::mat3 matrix){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniformMatrix3fv(id,location,1,GL_FALSE,glm::value_ptr(matrix));
            #else
            glUseProgram(id);
            glUniformMatrix3fv(location,1,GL_FALSE,glm::value_ptr(matrix));
            #endif
        }
        void uniformMatrix4fv(const char* name,glm::mat4 matrix){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniformMatrix4fv(id,location,1,GL_FALSE,glm::value_ptr(matrix));
            #else
            glUseProgram(id);
            glUniformMatrix4fv(location,1,GL_FALSE,glm::value_ptr(matrix));
            #endif
        }
        #ifdef CGL_BINDLESS_SUPPORT
        void uniformHandleui64ARB(const char* name,GLuint64 handle){
            GLint location = glGetUniformLocation(id,name);
            #ifdef CGL_USE_DSA
            glProgramUniformHandleui64ARB(id,location,handle);
            #else
            glUseProgram(id);
            glUniformHandleui64ARB(location,handle);
            #endif
        }
        #endif
};
#endif
