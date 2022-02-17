#ifndef CGL_UTILS
#define CGL_UTILS
#include<iostream>
#include<fstream>
#include<sys/stat.h>

std::string loadStringFile(const char* fname){
    std::ifstream file;
    file.open(fname);
    std::string st;
    char ch;
    if(!file){
        return NULL;
    }
    while(!file.eof()){
        file >> std::noskipws >> ch;
        st += ch;
    }
    file.close();
    return st;
}
bool fileexists(std::string file){
    struct stat trash;
    return stat(file.c_str(),&trash) != -1;
}
#endif
