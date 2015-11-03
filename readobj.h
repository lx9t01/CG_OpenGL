#pragma once
#include <string>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include </usr/local/include/Eigen/Dense>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <Eigen/Dense>
#endif

using namespace std;


struct RGB{
    float R,G,B;
};


struct vertex {
  float x, y, z;
  vertex(float x1, float y1, float z1) {
    x = x1;
    y = y1;
    z = z1;
  }
};

struct vertexn{
  float x, y, z;
  vertexn(float x1, float y1, float z1) {
    x = x1;
    y = y1;
    z = z1;
  }
};

struct face {
  int a, b, c;
  int an, bn, cn;
  RGB a_color, b_color, c_color;
  face(int a1, int a2, int b1, int b2, int c1, int c2) {
    a = a1;
    b = b1;
    c = c1;
    an = a2;
    bn = b2;
    cn = c2;
  }
};

struct Point_Light
{
    float position[4]; //xyzw
    float color[3]; //rgb range?
    float attenuation_k;
};


struct Transforms
{
    float translation[3]; //xyz
    float rotation[3];
    float scaling[3];
    float rotation_angle; // angle in deg
};




class GraphObj {
 public:
  GraphObj(char* s);
  GraphObj();
  bool readObj(char const* s);
    
    //readobj.cpp
  string getName();
  unsigned int getNumber();
  bool setName(string s);
  bool setNumber(unsigned int n);
  string getPrintName();
  bool setPrintName(string s);
  int dupMore();
  int getDupNumber();
    
    //readtrans.cpp
  vector<vertex> getVertex();
  vector<face> getFace();
  bool setVertex(vector<vertex> v1);
  bool setFace(vector<face> f1);//color render
  vector<vertexn> getVertexN();
  bool setVertexN(vector<vertexn>);
    
    //readobj.cpp
  bool setAmbient(float r, float g,float b);
  bool setDiffuse(float r, float g, float b);
  bool setSpecular(float r, float g, float b);
  bool setShininess(float p);
  float ambient[3];
  float diffuse[3];
  float specular[3];
  float shininess;
  vector<Transforms> transform_sets;
  vector<vertex> vertex_buffer;
  vector<vertexn> vertexN_buffer;


 private:
  string objname;
  string printname;
  unsigned int objnumber;
  int dupnumber=0;
  vector<vertex> v;
  vector<face> f;
  vector<vertexn> vn;

    
    
};