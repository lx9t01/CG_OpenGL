#include <iostream>
#include <fstream>
#include <sstream>
#include "readobj.h"
#include <math.h>


using namespace std;
using namespace Eigen;

Matrix4f GraphObj::readTrans(char const* s){
	ifstream transfile(s);
	float a[4];
	Matrix4f m=Matrix4f::Zero(4,4);
	Matrix4f mout=Matrix4f::Zero(4,4);
	string line, substring;
	vector<string> v;

	mout(0,0)=1;
	mout(1,1)=1;
	mout(2,2)=1;
	mout(3,3)=1;

	// cout << "This is out matrix m.\n" << mout << endl;

	if (transfile.is_open()){
		while (getline(transfile,line)){
			//cout << line << endl;
			stringstream ss(line);
			while (getline(ss, substring, ' ')){
				if (substring=="t"){
					for (int i=0; i<3; i++){
						getline(ss, substring, ' ');
						stringstream(substring) >> a[i];

					}
					m=GraphObj::translation(a[0], a[1], a[2]);
					//cout << m << endl;
					mout=m*mout;

				}else if (substring=="r"){
					for (int i=0; i<4; i++){
						getline(ss, substring, ' ');
						stringstream(substring) >> a[i];

					}
					m=GraphObj::rotation(a[0], a[1], a[2], a[3]);
					//cout << m <<endl;
					mout=m*mout;

				}else if (substring=="s"){
					for (int i=0; i<3; i++){
						getline(ss, substring, ' ');
						stringstream(substring) >> a[i];
					}
					m=GraphObj::scaling(a[0], a[1], a[2]);
					//cout << m <<endl;
					mout=m*mout;

				}else cout << "No valid trans!\n";
			}
			


		}
		

		transfile.close();
	}else {
		cout << "TransFile not Open.\n";
		return mout;
	}
	cout<< "This is the final Matrix:\n";
	cout << mout << endl;
	return mout;
}

Matrix4f GraphObj::rotation (float a, float b, float c, float d){
	Matrix4f mr=Matrix4f::Zero(4,4);
    float norm;
    norm=sqrt(a*a+b*b+c*c);//To normalize the rotation vector.
    a=a/norm;
    b=b/norm;
    c=c/norm;
    
	mr(0,0)=a*a+(1-a*a)*cos(d);
	mr(1,1)=b*b+(1-b*b)*cos(d);
	mr(2,2)=c*c+(1-c*c)*cos(d);
	mr(3,3)=1;
	mr(0,1)=a*b*(1-cos(d))-c*sin(d);mr(1,0)=a*b*(1-cos(d))+c*sin(d);
	mr(0,2)=a*c*(1-cos(d))+b*sin(d);mr(2,0)=a*c*(1-cos(d))-b*sin(d);
	mr(1,2)=b*c*(1-cos(d))-a*sin(d);mr(2,1)=b*c*(1-cos(d))+a*sin(d);
	return mr;
}

Matrix4f GraphObj::translation (float a, float b, float c){
	Matrix4f mt=Matrix4f::Zero(4,4);
	mt(1,1)=1;mt(2,2)=1;mt(3,3)=1;mt(0,0)=1;
	mt(0,3)=a;mt(1,3)=b;mt(2,3)=c;
	return mt;

}

Matrix4f GraphObj::scaling (float a, float b, float c){
	Matrix4f ms=Matrix4f::Zero(4,4);
	ms(0,0)=a;ms(1,1)=b;ms(2,2)=c;ms(3,3)=1;
	return ms;

}

vector<vertex> GraphObj::getVertex(){
    
    return v;
}

vector<face> GraphObj::getFace(){
    return f;
}

bool GraphObj::setVertex(vector<vertex> v1){
    v=v1;
    
    return true;
};

bool GraphObj::setFace(vector<face> f1){
    f=f1;
    return true;
}


bool GraphObj::setVertex_NDC(vector<vertex> v1){
    v_NDC=v1;
    
    return true;
};

vector<vertex> GraphObj::getVertex_NDC(){
    
    return v_NDC;
}


vector<vertexn> GraphObj::getVertexN(){
    return vn;
}


bool GraphObj::setVertexN(vector<vertexn> v1){
    vn=v1;
    return true;
}
