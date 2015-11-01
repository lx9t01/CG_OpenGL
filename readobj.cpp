#include <iostream>
#include <sstream>
#include <fstream>
#include "readobj.h"

using namespace std;
using Eigen::MatrixXd;

void split(string s, int &s1, int &s2);

GraphObj::GraphObj(char* s){
  ifstream objfile(s);
  string ch;
  string x, y, z;
    string str;
    int a1,b1,c1,a2,b2,c2;
    
  if (objfile.is_open()) {
    while (!objfile.eof()) {
      objfile >> ch >> x >> y >> z;
      if (ch == "v") {
        v.push_back(vertex(stof(x), stof(y), stof(z), 1));
      } else if (ch == "f") {
          split(x,a1,a2);
          split(y,b1,b2);
          split(z,c1,c2);
//          cout<<"ao: "<<a1<<" "<<a2<<endl;
//          cout<<"bo: "<<b1<<" "<<b2<<endl;
//          cout<<"co: "<<c1<<" "<<c2<<endl;
          f.push_back(face(a1, a2, b1, b2, c1, c2));
      } else if(ch == "vn"){
          vn.push_back(vertexn(stof(x), stof(y), stof(z), 1));
      }
        else
        cout << "Fail to read obj line";
      }
    
    f.pop_back(); // one more last line
    objfile.close();
  } else {
    cout << "File not open.\n";
  }
}

GraphObj::GraphObj(){
  
}

bool GraphObj::readObj(char const* s) {
    ifstream objfile(s);
    string ch;
    string x, y, z;
    string str;
    int a1,b1,c1,a2,b2,c2;
    
    if (objfile.is_open()) {
        while (!objfile.eof()) {
            objfile >> ch >> x >> y >> z;
            if (ch == "v") {
                v.push_back(vertex(stof(x), stof(y), stof(z), 1));
            } else if (ch == "f") {
                split(x,a1,a2);
                split(y,b1,b2);
                split(z,c1,c2);
                //cout<<"ao: "<<a1<<" "<<a2<<endl;
                f.push_back(face(a1, a2, b1, b2, c1, c2));
            } else if(ch == "vn"){
                vn.push_back(vertexn(stof(x), stof(y), stof(z), 1));
            }
            else
                cout << "Fail to read obj line";
        }
        
        f.pop_back(); // one more last line
        objfile.close();
    } else {
        cout << "File not open.\n";
        return false;
    }
  return true;
}

string GraphObj::getName(){
  return objname;

}

unsigned int GraphObj::getNumber(){
  return objnumber;

}

bool GraphObj::setName(string s){
  objname=s;
  return true;
}

bool GraphObj::setNumber(unsigned int n){
  objnumber=n;
  return true;
}

bool GraphObj::setPrintName(string s){
    printname=s;
    return true;
}

string GraphObj::getPrintName(){
    return printname;
}

int GraphObj::dupMore(){
    dupnumber++;
    return dupnumber;
}

int GraphObj::getDupNumber(){
    return dupnumber;
}

void split(string s, int &s1, int &s2){// to split vertex and norm vectors
    int l;
    l=s.length();
    size_t pos=s.find("//");
    string sub1=s.substr(0,pos);
    string sub2=s.substr(pos+2,l);
    s1=atoi(sub1.c_str());
    s2=atoi(sub2.c_str());
    
}

bool GraphObj::setAmbient(float r, float g,float b){
    ambient[0]=r;
    ambient[1]=g;
    ambient[2]=b;
    return true;
}

bool GraphObj::setDiffuse(float r, float g, float b){
    diffuse[0]=r;
    diffuse[1]=g;
    diffuse[2]=b;
    return true;
}

bool GraphObj::setSpecular(float r, float g, float b){
    specular[0]=r;
    specular[1]=g;
    specular[2]=b;
    return true;
}

bool GraphObj::setShininess(float p){
    shininess=p;
    return true;
}

