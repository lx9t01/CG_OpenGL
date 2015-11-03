
#include "readobj.h"


using namespace std;
using namespace Eigen;


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


vector<vertexn> GraphObj::getVertexN(){
    return vn;
}


bool GraphObj::setVertexN(vector<vertexn> v1){
    vn=v1;
    return true;
}
