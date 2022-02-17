#ifndef CGL_CAMERA
#define CGL_CAMERA

#include<cmath>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
class Camera{
    public:
        double pitch,yaw,roll;
        glm::vec3 pos;
        Camera() = default;
        Camera(glm::vec3 pos,double pitch=0.0,double yaw=0.0,double roll=0.0){
            this->pos = pos;
            this->pitch = pitch;
            this->yaw = yaw;
            this->roll = roll;
        }
        glm::vec3 inline worldup(){
            double rroll = glm::radians(roll);
            return glm::vec3(sin(rroll),cos(rroll),0);
        }
        glm::mat4 viewmatrix(){
            glm::mat4 trans = glm::mat4(1.0);
            trans = glm::rotate(trans,(float)-glm::radians(pitch),glm::vec3(1.0,0.0,0.0));
            trans = glm::rotate(trans,(float)glm::radians(yaw+90),glm::vec3(0.0,1.0,0.0));
            trans = glm::rotate(trans,(float)-glm::radians(roll),direction());
            trans = glm::translate(trans,-pos);
            return trans;
        }
        glm::vec3 direction(){
            double ryaw = glm::radians(yaw);
            double rpitch = glm::radians(pitch);
            return glm::normalize(glm::vec3(cos(ryaw)*cos(rpitch),sin(rpitch),sin(ryaw)*cos(rpitch)));
        }
        glm::vec3 xzfront(){
            double ryaw = glm::radians(yaw);
            return glm::vec3(cos(ryaw),0,sin(ryaw));
        }
        glm::vec3 xzright(){
            double ryaw = glm::radians(yaw);
            return glm::vec3(-sin(ryaw),0,cos(ryaw));
        }
        glm::vec3 right(){
            glm::mat4 trans = glm::mat4(1.0);
            trans = glm::rotate(trans,(float)glm::radians(yaw+90),glm::vec3(0.0,1.0,0.0));
            trans = glm::rotate(trans,(float)-glm::radians(roll),direction());
            return glm::mat3(trans)*glm::vec3(1.0,0.0,0.0);
        }
        glm::vec3 up(){
            return glm::cross(direction(),right());
        }
};
#endif
