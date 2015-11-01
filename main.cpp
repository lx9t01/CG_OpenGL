//
//  main.cpp
//  hw3
//
//  Created by 徐海翔 on 15/10/10.
//  Copyright (c) 2015年 徐海翔. All rights reserved.
//

#include "readobj.h"
#include <iostream>
#include </usr/local/include/Eigen/Dense>
#include <sstream>
#include <fstream>

using namespace std;
using namespace Eigen;

int xres,yres;
int mode=0;//initially gourand shading
GraphObj magic(GraphObj obj);//used to perform the trans/rotation/scaling to obj
GraphObj magic2(GraphObj obj, Matrix4f a);// used for world2cam, also used for cam2ndc
GraphObj paint_vertex(GraphObj obj, vertex ecam, vector<light> lightsource);//after trans, before world2cam, set color to each vertex

int First_Octant_Bresenham(int x0,int y0,int x1,int y1, int** grid);//previous frame

void Raster_Colored_Triangle(face f, vertex a, vertex b, vertex c, RGB** grid, float** buffer);
void Raster_Colored_Triangle_Phong(face f,vertex a, vertex b, vertex c, vector<vertexn> vn, RGB** grid, float** buffer, Matrix4f P, Matrix4f camera_matrix_P, GraphObj obj, vertex ecam, vector<light> lights);

int min(int a, int b, int c);
int max(int a ,int b, int c);

void fill(int x, int y, RGB** grid, float* color);

int main(int argc, char** argv) {
    ifstream graphfile(argv[1]);
    xres=atoi(argv[2]);//atoi
    yres=atoi(argv[3]);
    mode=atoi(argv[4]);
    
    vector<light> lightsource;
    vector<GraphObj> graph;//set of graph objs

    string line;
    string objname,objfilename;
    unsigned int flag=0;
    unsigned int number=0;
    float a[4];
    int i=0;
    string nametemp, stringtemp, command;
    int numtemp=0;
    Matrix4f m=Matrix4f::Identity(4,4);
    
    float cam_p[3]={0,0,0};
    float cam_o[4]={0,0,0,0};
    float n=0, f=0, l=0, r=0, t=0, b=0;
    
    light light1;
    
    if (graphfile.is_open()){
        while(getline(graphfile,line)){//read each line
            //read camera
            if (line.size() >0 && flag ==0){
                getline(graphfile, line);//camera:
                stringstream ss(line);
                getline(ss, command, ' ');
                for (int i=0;i<3;i++){
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>cam_p[i];
                }
                
                getline(graphfile, line);
                stringstream ss1(line);
                getline(ss1, command, ' ');
                for (int i=0;i<4;i++){
                    getline(ss1, stringtemp, ' ');
                    stringstream(stringtemp)>>cam_o[i];
                }
                
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>n;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>f;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>l;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>r;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>t;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>b;
                
            }else if (line.size()==0 && flag==0){//read in the obj files
                flag=1;
                getline(graphfile,line);
            }
            
            // read in lights
            if(line.size()!=0 && flag==1){
                string lighttemp;
                stringstream ss_l(line);
                getline(ss_l, stringtemp, ' ');
                
                getline(ss_l, stringtemp, ',');// , as separation. xyz
                stringstream ss_l1(stringtemp);
                for (int i=0;i<3;i++){
                    getline(ss_l1, lighttemp, ' ');
                    stringstream(lighttemp)>>light1.pos[i];
                }
                
                getline(ss_l, stringtemp, ',');// , as separation. rgb
                stringstream ss_l2(stringtemp);
                getline(ss_l2, lighttemp, ' ');// avoid one more ' '
                for (int i=0;i<3;i++){
                    getline(ss_l2, lighttemp, ' ');
                    stringstream(lighttemp)>>light1.color[i];
                }
                
                getline(ss_l, stringtemp);// , as separation. k
                stringstream ss_l3(stringtemp);
                ss_l3>>light1.k;
                
                lightsource.push_back(light1);
            }else if(line.size()==0 && flag==1){
                flag=2;
                getline(graphfile,line);
            }
            
            
            if(flag==2&&line.size()>0&&line!="objects:"){
                stringstream ss(line);
                if (getline(ss, objname,' ')){
                    getline(ss, objfilename);
                    
                    char* cstr = new char[objfilename.length()+1];
                    strcpy(cstr, objfilename.c_str());
                    
                    GraphObj obj(cstr);//define a object named 'obj'
                    delete cstr;
                    stringstream ss2;
                    ss2<<objname<<"_copy_1";
                    obj.setPrintName(ss2.str());
                    obj.setName(objname);
                    obj.setNumber(number);//total number of objs
                    number++;
                    graph.push_back(obj);
                }
            }else if (flag==2&&line.size()==0){
                flag=3;
            }
            
            if(flag==3&&line.size()==0){
                getline(graphfile,line);
                stringtemp=line;
                for (numtemp=0;numtemp<graph.size();numtemp++){
                    if (stringtemp==graph[numtemp].getName()){
                        graph[numtemp].dupMore();
                        if (graph[numtemp].getDupNumber()>1){
                            GraphObj newobj;
                            newobj=graph[numtemp];
                            stringstream ss1;
                            ss1<<graph[numtemp].getName()<<"_copy_"<<graph[numtemp].getDupNumber();
                            newobj.setPrintName(ss1.str());
                            newobj.setName(ss1.str());
                            newobj.setNumber(number);
                            numtemp=number;//IMPORTANT!! record the index of obj in vector<obj>
                            number++;
                            newobj.matrix=Matrix4f::Identity(4,4);
                            graph.push_back(newobj);
                        }
                        break;
                    }
                }//for
            }else if(flag==3&&line.size()>0){
                stringstream ss(line);
                getline(ss, stringtemp, ' ');
                float r,g,b,p;
                if (stringtemp=="ambient"){
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>r;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>g;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>b;
                    graph[numtemp].setAmbient(r, g, b);
                }else if(stringtemp=="diffuse"){
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>r;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>g;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>b;
                    graph[numtemp].setDiffuse(r, g, b);
                }else if (stringtemp=="specular"){
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>r;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>g;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>b;
                    graph[numtemp].setSpecular(r, g, b);
                }else if(stringtemp=="shininess"){
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>p;
                    graph[numtemp].setShininess(p);
                }else if (stringtemp=="t"){
                    for (i=0;i<3;i++){
                        getline(ss, stringtemp,' ');
                        stringstream(stringtemp)>>a[i];
                    }
                    m=graph[numtemp].translation(a[0],a[1],a[2]);

                    graph[numtemp].matrix=m*graph[numtemp].matrix;
                    graph[numtemp].inv_matrix=graph[numtemp].matrix.inverse();
                
                    
                }else if (stringtemp=="r"){
                    for (i=0;i<4;i++){
                        getline(ss, stringtemp,' ');
                        stringstream(stringtemp)>>a[i];
                    }
                    m=graph[numtemp].rotation(a[0],a[1],a[2],a[3]);
                    graph[numtemp].matrix=m*graph[numtemp].matrix;
                    graph[numtemp].inv_matrix=graph[numtemp].matrix.inverse();
                    graph[numtemp].n_matrix=graph[numtemp].n_matrix*m;//for norm vector's rotation
                    
                }else if (stringtemp=="s"){
                    for (i=0;i<3;i++){
                        getline(ss, stringtemp, ' ');
                        stringstream(stringtemp)>>a[i];
                    }
                    m=graph[numtemp].scaling(a[0],a[1],a[2]);
                    graph[numtemp].matrix=m*graph[numtemp].matrix;
                    graph[numtemp].inv_matrix=graph[numtemp].matrix.inverse();
                    graph[numtemp].n_matrix=m*graph[numtemp].n_matrix;//for norm vector's scale
                    
                }else{
                    cout << "No such transform!\n";
                }//if flag=2 && not empty line
                graph[numtemp].inv_n_matrix=graph[numtemp].n_matrix.inverse().transpose();
                //calculate X=(inv(S*R))^T;
            }
            
        }
        //end of read
        //end of output obj vertex
        
    }else{
        cout <<"Graph file not open.\n";
        return 1;
    }//if open graph file
    
    //==================perform trasfroamtion======================
    vector<GraphObj> graph_after_trans;//set of graph objs
            for (numtemp=0;numtemp<graph.size();numtemp++){
                GraphObj objout;
                objout=magic(graph[numtemp]);// including the trans of norm vectors
                graph_after_trans.push_back(objout);
                graph_after_trans[numtemp].matrix=graph[numtemp].matrix;
                graph_after_trans[numtemp].inv_matrix=graph[numtemp].inv_matrix;
                graph_after_trans[numtemp].setPrintName(graph[numtemp].getPrintName());
            }
    
    
    
    //==================give color to each vertex==================
    vector<GraphObj> graph_after_color;
    vertex e_cam(cam_p[0],cam_p[1],cam_p[2],1);
    
    for (int i=0;i<graph_after_trans.size();i++){
        GraphObj objcolor;
        objcolor=paint_vertex(graph_after_trans[i], e_cam, lightsource);

        graph_after_color.push_back(objcolor);
        graph_after_color[i].matrix=graph[i].matrix;
        graph_after_color[i].inv_matrix=graph[i].inv_matrix;
        graph_after_color[i].setPrintName(graph[i].getPrintName());
        
    }

    
    //==================camera=====================================
    camera c1;
    Matrix4f camera_matrix_P=c1.setInvC(cam_p, cam_o);
    vector<GraphObj> graph_after_camera;//set of graph objs
    //output objs' vertex
    for (numtemp=0;numtemp<graph_after_color.size();numtemp++){
        GraphObj objout;
        objout=magic2(graph_after_color[numtemp],camera_matrix_P);
        vector<vertex> v1;
        v1=objout.getVertex();
        objout.setVertex(v1);//redundant probably
        graph_after_camera.push_back(objout);//inportant, save for next step
        graph_after_camera[numtemp].matrix=graph[numtemp].matrix;
        graph_after_camera[numtemp].inv_matrix=graph[numtemp].inv_matrix;
        graph_after_camera[numtemp].setPrintName(graph[numtemp].getPrintName());
    }
    //end of output, Section 2 finished

    
    //==============cam2ndc=======================================
    Matrix4f P=Matrix4f::Zero(4, 4);//P: Camera Space to NDC;
    vector<GraphObj> graph_after_NDC;//set of graph objs
    P<<2*n/(r-l),0,(r+l)/(r-l),0,
    0,2*n/(t-b),(t+b)/(t-b),0,
    0,0,-(f+n)/(f-n),-2*f*n/(f-n),
    0,0,-1,0;
    //project to NDC
    for (numtemp=0;numtemp<graph_after_camera.size();numtemp++){
        GraphObj objout;
        objout=magic2(graph_after_camera[numtemp],P);
        vector<vertex> v1;
        v1=objout.getVertex();
        objout.setVertex(v1);
        objout.setVertex_NDC(v1);
        graph_after_NDC.push_back(objout);
        graph_after_NDC[numtemp].matrix=graph[numtemp].matrix;
        graph_after_NDC[numtemp].inv_matrix=graph[numtemp].inv_matrix;
        graph_after_NDC[numtemp].setPrintName(graph[numtemp].getPrintName());

    }
    //end of output NDC, Section 3 finished
    //not check whether inside the frustum yet

    
    //==============ndc2screen====================================

    RGB** grid;
    //grid matrix!----------------------------------
    
    grid=new RGB*[yres];
    for (int i=0;i<yres;i++){
            grid[i]=new RGB[xres];
    }

    for (int i=0;i<yres;i++){
    	for (int j=0;j<xres;j++){
    		grid[i][j].R=0;
    		grid[i][j].G=0;
    		grid[i][j].B=0;
    	}
    }
    
    
    //Proj to 2D
    for (int i=0;i<graph_after_NDC.size();i++){
        GraphObj objtemp=graph_after_NDC[i];
        vector<vertex> v1;
        vector<face> f1;
        vector<vertexn> vn1;
        vector <int> x;
        vector <int> y;
        
        //===================new===========================
        f1=objtemp.getFace();
        v1=objtemp.getVertex();
        vn1=objtemp.getVertexN();
        
        
        // initialize buffer
        float** buffer;
        buffer=new float*[yres];
        for (int j=0;j<yres;j++){
            buffer[j]=new float[xres];
        }
        for (int k=0;k<yres;k++){
            for (int l=0;l<xres;l++)
                buffer[k][l]=99999999;
        }
        
        for (int j=0;j<f1.size();j++){
            int a,b,c;
            a=f1[j].a-1;
            b=f1[j].b-1;
            c=f1[j].c-1;
            
//            vn.x=(vn1[f1[j].an-1].x+vn1[f1[j].bn-1].x+vn1[f1[j].cn-1].x)/3;
//            vn.y=(vn1[f1[j].an-1].y+vn1[f1[j].bn-1].y+vn1[f1[j].cn-1].y)/3;
//            vn.z=(vn1[f1[j].an-1].z+vn1[f1[j].bn-1].z+vn1[f1[j].cn-1].z)/3;
            // use the avg face normal for each point on that face
            
            if (mode==0){
                Raster_Colored_Triangle(f1[j], v1[a], v1[b], v1[c], grid, buffer);// To be finished

            }else if (mode==1){
                Raster_Colored_Triangle_Phong(f1[j],v1[a], v1[b], v1[c], vn1, grid, buffer, P, camera_matrix_P, objtemp, e_cam, lightsource);
                //use vn vector to find the right vn, and use alpha, beta, gamma calculated later to obtain vn for specific point

            }else{
                
            }
            
            
        }
        
        //==================previous=======================
        /*
        
        v1=objtemp.getVertex();
        f1=objtemp.getFace();
        for (int j=0;j<v1.size();j++){
            int temp=0;
            temp=(int)xres/2*v1[j].x/v1[j].w;
            x.push_back(temp);
            //cout<<temp<<"\n";
            temp=(int)yres/2*v1[j].y/v1[j].w;
            y.push_back(temp);
            //cout<<temp<<"\n";
            
        }
        for (int j=0;j<f1.size();j++)
        {
            int a,b,c;
            // to locate the three verteces corresponding to the j th face
            a=f1[j].a-1;//#of vertex starts from 0;
            b=f1[j].b-1;
            c=f1[j].c-1;
            First_Octant_Bresenham(x[a], y[a], x[b], y[b], grid);
            First_Octant_Bresenham(x[b], y[b], x[c], y[c], grid);
            First_Octant_Bresenham(x[a], y[a], x[c], y[c], grid);
            
        }
         */
    }
    
    //Output Grid
    //write files=========================================
    ofstream outfile;
    outfile.open("testout.ppm");
    outfile<<"P3\n";
    outfile<<xres<<" "<<yres<<"\n";
    outfile<<"255\n";
    
    /*
    for (int i=yres-1;i>=0;i--){//!!!!!bug probably
        for (int j=0;j<xres;j++){
            cout<<grid[i][j]<<" ";
            if (grid[i][j]==1)
                outfile<< "255 255 255"<<"\n";
            else
                outfile<< "0 0 0"<<"\n";
        }
        cout<<"\n";
    }
    */
    
    for (int i=yres-1;i>=0;i--){
        for (int j=0;j<xres;j++){
            outfile<<grid[i][j].R<<" "<<grid[i][j].G<<" "<<grid[i][j].B<<"\n";
        }
    }
    
    outfile.close();

    for (int i=0;i<yres;i++){
        delete[] grid[i];
    }
    delete[] grid;
    //-----------------------------------------------------------
    

    return 0;
}
//=================================================================

GraphObj magic(GraphObj obj){//TO perform trans to vertex and norm vertex
    vector<vertex> v1,v2;
    vector<vertexn> vn1,vn2;//norm vectors
    v1=obj.getVertex();
    vn1=obj.getVertexN();
    
    for (int i=0;i<v1.size();i++){
        Vector4f a(4);
        a(0)=v1[i].x;
        a(1)=v1[i].y;
        a(2)=v1[i].z;
        a(3)=v1[i].w;
        a=obj.matrix*a;
        v2.push_back(vertex(a(0),a(1),a(2),a(3)));
    }
    
    for (int j=0;j<vn1.size();j++){
        Vector4f b(4);
        b(0)=vn1[j].x;
        b(1)=vn1[j].y;
        b(2)=vn1[j].z;
        b(3)=vn1[j].w;
        b=obj.inv_n_matrix*b;//QUESTOIN!!!!!! why inverse?
        vn2.push_back(vertexn(b(0),b(1),b(2),b(3)));
    }
    obj.setVertex(v2);
    obj.setVertexN(vn2);

    return obj;
}

GraphObj magic2(GraphObj obj, Matrix4f a){//also used for cam2NDC
    vector<vertex> v1=obj.getVertex();
    vector<vertex> v2;
    Vector4f b(4);
    
    for (int i=0;i<v1.size();i++){
//        cout<<v1[i].x<<" "<<v1[i].y<<" "<<v1[i].z<<" "<<v1[i].w<<endl;
        b(0)=v1[i].x;
        b(1)=v1[i].y;
        b(2)=v1[i].z;
        b(3)=v1[i].w;

        b=a*b;
        v2.push_back(vertex(b(0),b(1),b(2),b(3)));

    }
    obj.setVertex(v2);
    
    return obj;
    
}

GraphObj paint_vertex(GraphObj obj, vertex e_cam, vector<light> lightsource){
    
    Vector3f c_lighting;

    vector<face> fcolor;
    vector<vertex> vcolor;
    vector<vertexn> vncolor;
    fcolor=obj.getFace();
    vcolor=obj.getVertex();
    vncolor=obj.getVertexN();
    
    for (int j=0;j<fcolor.size();j++){
    	// made one mistake here, for vncolor, have to refer to fcolor[j].aN-1!
        
        c_lighting=lighting(vcolor[fcolor[j].a-1], vncolor[fcolor[j].an-1], obj.diffuse, obj.ambient, obj.specular, obj.shininess, e_cam, lightsource);
        fcolor[j].a_color.R=c_lighting(0);
        fcolor[j].a_color.G=c_lighting(1);
        fcolor[j].a_color.B=c_lighting(2);
//        temprgb.R=c_lighting(0);
//        temprgb.G=c_lighting(1);
//        temprgb.B=c_lighting(2);
//        vcolor[fcolor[j].a-1].rgb.push_back(temprgb);
//        vcolor[fcolor[j].a-1].R=c_lighting(0);
//        vcolor[fcolor[j].a-1].G=c_lighting(1);
//        vcolor[fcolor[j].a-1].B=c_lighting(2);
        
        
        c_lighting=lighting(vcolor[fcolor[j].b-1], vncolor[fcolor[j].bn-1], obj.diffuse, obj.ambient, obj.specular, obj.shininess, e_cam, lightsource);
        fcolor[j].b_color.R=c_lighting(0);
        fcolor[j].b_color.G=c_lighting(1);
        fcolor[j].b_color.B=c_lighting(2);
//        temprgb.R=c_lighting(0);
//        temprgb.G=c_lighting(1);
//        temprgb.B=c_lighting(2);
//        vcolor[fcolor[j].b-1].rgb.push_back(temprgb);
//        vcolor[fcolor[j].b-1].R=c_lighting(0);
//        vcolor[fcolor[j].b-1].G=c_lighting(1);
//        vcolor[fcolor[j].b-1].B=c_lighting(2);
        
        c_lighting=lighting(vcolor[fcolor[j].c-1], vncolor[fcolor[j].cn-1], obj.diffuse, obj.ambient, obj.specular, obj.shininess, e_cam, lightsource);
        fcolor[j].c_color.R=c_lighting(0);
        fcolor[j].c_color.G=c_lighting(1);
        fcolor[j].c_color.B=c_lighting(2);
//        temprgb.R=c_lighting(0);
//        temprgb.G=c_lighting(1);
//        temprgb.B=c_lighting(2);
//        vcolor[fcolor[j].c-1].rgb.push_back(temprgb);
//        vcolor[fcolor[j].c-1].R=c_lighting(0);
//        vcolor[fcolor[j].c-1].G=c_lighting(1);
//        vcolor[fcolor[j].c-1].B=c_lighting(2);
        
    }
    
    obj.setFace(fcolor);
    
    
    return obj;
}


int First_Octant_Bresenham(int x0,int y0,int x1,int y1, int **grid){
    int e=0;
    int dx,dy=0;
    if(x1<x0){
        int temp;
        temp=x1;x1=x0;x0=temp;
        temp=y1;y1=y0;y0=temp;
    }// swap, x1>=x0
    dx=x1-x0;
    if (y1>=y0){
        dy=y1-y0;
        if (dy>dx){
            int x=x0;
            for (int y=y0;y<y1;y++){
                if ((abs(x)<xres/2)&&(abs(y)<yres/2)){
                    grid[y+yres/2][x+xres/2]=1;
                    if(2*e+dx<dy){
                        e=e+dx;
                    }else{
                        e=e+dx-dy;
                        x=x+1;
                    }
                }else
                    break;
            }
        }else{
            int y=y0;
            for (int x=x0;x<x1;x++){
                if ((abs(x)<xres/2)&&(abs(y)<yres/2)){
                    grid[y+yres/2][x+xres/2]=1;
                    if (2*(e+dy)<dx){
                        e=e+dy;
                    }else {
                        e=e+dy-dx;
                        y=y+1;
                    }
                }else
                    break;
            }
        }
    }else{
        dy=y0-y1;
        if (dy>dx){
            int x=x1;
            for (int y=y1;y<y0;y++){
                if((abs(x)<xres/2)&&(abs(y)<yres/2)){
                    grid[y+yres/2][x+xres/2]=1;
                    if (2*e+dx<dy){
                        e=e+dx;
                    }else{
                        e=e+dx-dy;
                        x=x-1;
                    }
                }else
                    break;
            }
        }else{
            int y=y0;
            for (int x=x0;x<x1;x++){
                if((abs(x)<xres/2)&&(abs(y)<yres/2)){
                    grid[y+yres/2][x+xres/2]=1;
                    if (2*(e+dy)<dx){
                        e=e+dy;
                    }else {
                        e=e+dy-dx;
                        y=y-1;
                    }
                }else
                    break;
            }
            
        }
        
    }
    
    return 0;
}


void Raster_Colored_Triangle(face f,vertex a, vertex b, vertex c, RGB** grid, float** buffer){
    Vector3f c_b, a_b,cross;
    
    c_b<<c.x-b.x,c.y-b.y,c.z-b.z;
    a_b<<a.x-b.x,a.y-b.y,a.z-b.y;
    cross=c_b.cross(a_b);
    //perform Backface Culling
    if (cross(2)<0){
        return;
    }

    
    int xa,ya,xb,yb,xc,yc;
    xa=(int)xres/2*a.x/a.w;ya=(int)yres/2*a.y/a.w;
    xb=(int)xres/2*b.x/b.w;yb=(int)yres/2*b.y/b.w;
    xc=(int)xres/2*c.x/c.w;yc=(int)yres/2*c.y/c.w;
    
    int xmin,xmax,ymin,ymax;
    xmin=min(xa, xb, xc);
    xmax=max(xa,xb,xc);
    ymin=min(ya,yb,yc);
    ymax=max(ya,yb,yc);
    vertex NDC(0,0,0,1);
    
    for (int x=xmin;x<xmax;x++){
        for (int y=ymin;y<ymax;y++){
            float alpha, beta, gamma;
            alpha=compute_alpha(xa,ya,xb,yb,xc,yc,x,y);
            beta=compute_beta(xa,ya,xb,yb,xc,yc,x,y);
            gamma=compute_gamma(xa,ya,xb,yb,xc,yc,x,y);
            //cout<<"alpha: "<<alpha<<" "<<beta<<" "<<gamma<<"\n";
            
            if(alpha>=0 && alpha<=1 && beta>=0 && beta<=1 && gamma>=0 && gamma<=1){
                NDC.x=alpha*a.x/a.w+beta*b.x/b.w+gamma*c.x/c.w;
                NDC.y=alpha*a.y/a.w+beta*b.y/b.w+gamma*c.y/c.w;
                NDC.z=alpha*a.z/a.w+beta*b.z/b.w+gamma*c.z/c.w;
                
                if (NDC.x>=-1 && NDC.x<=1 && NDC.y>=-1 && NDC.y<=1 && NDC.z>=-1 && NDC.z<=1){//check if within NDC box
                    if (NDC.z<=buffer[y+yres/2][x+xres/2]){
                        buffer[y+yres/2][x+xres/2]=NDC.z;
                        //perform depth buffering
                            //Gourand Shading
                            float R,G,B;
                            float color[3];
                            //to store rgb values of verteces in different face(with different norm) differently
                            R=alpha*f.a_color.R+beta*f.b_color.R+gamma*f.c_color.R;
                            G=alpha*f.a_color.G+beta*f.b_color.G+gamma*f.c_color.G;
                            B=alpha*f.a_color.B+beta*f.b_color.B+gamma*f.c_color.B;
                            //cout<<f.a_color.R<<" "<<f.a_color.G<<" "<<f.a_color.B<<endl;
                            color[0]=R;color[1]=G;color[2]=B;
                            fill(x+xres/2,y+yres/2,grid, color);
                            
                        
                    }
                }
            }
        }
    }
    
}

void Raster_Colored_Triangle_Phong(face f,vertex a, vertex b, vertex c, vector<vertexn> vn, RGB** grid, float** buffer, Matrix4f P, Matrix4f camera_matrix_P, GraphObj obj, vertex ecam, vector<light> lights){
    Vector3f c_b, a_b,cross;
    
    c_b<<c.x-b.x,c.y-b.y,c.z-b.z;
    a_b<<a.x-b.x,a.y-b.y,a.z-b.y;
    cross=c_b.cross(a_b);
    //perform Backface Culling
    if (cross(2)<0){
        return;
    }
    
    
    int xa,ya,xb,yb,xc,yc;
    xa=(int)xres/2*a.x/a.w;ya=(int)yres/2*a.y/a.w;
    xb=(int)xres/2*b.x/b.w;yb=(int)yres/2*b.y/b.w;
    xc=(int)xres/2*c.x/c.w;yc=(int)yres/2*c.y/c.w;
    
    int xmin,xmax,ymin,ymax;
    xmin=min(xa, xb, xc);
    xmax=max(xa,xb,xc);
    ymin=min(ya,yb,yc);
    ymax=max(ya,yb,yc);
    vertex NDC(0,0,0,1);
    
    for (int x=xmin;x<xmax;x++){
        for (int y=ymin;y<ymax;y++){
            float alpha, beta, gamma;
            alpha=compute_alpha(xa,ya,xb,yb,xc,yc,x,y);
            beta=compute_beta(xa,ya,xb,yb,xc,yc,x,y);
            gamma=compute_gamma(xa,ya,xb,yb,xc,yc,x,y);
            //cout<<"alpha: "<<alpha<<" "<<beta<<" "<<gamma<<"\n";
            
            //calculate the new vn from alpha, beta, gamma
            vertexn vn_temp(0,0,0,1);
            vn_temp.x=alpha*vn[f.an-1].x+beta*vn[f.bn-1].x+gamma*vn[f.cn-1].x;
            vn_temp.y=alpha*vn[f.an-1].y+beta*vn[f.bn-1].y+gamma*vn[f.cn-1].y;
            vn_temp.z=alpha*vn[f.an-1].z+beta*vn[f.bn-1].z+gamma*vn[f.cn-1].z;
            
            if(alpha>=0 && alpha<=1 && beta>=0 && beta<=1 && gamma>=0 && gamma<=1){
                NDC.x=alpha*a.x/a.w+beta*b.x/b.w+gamma*c.x/c.w;
                NDC.y=alpha*a.y/a.w+beta*b.y/b.w+gamma*c.y/c.w;
                NDC.z=alpha*a.z/a.w+beta*b.z/b.w+gamma*c.z/c.w;
                
                if (NDC.x>=-1 && NDC.x<=1 && NDC.y>=-1 && NDC.y<=1 && NDC.z>=-1 && NDC.z<=1){
                    //check if within NDC box
                    if (NDC.z<=buffer[y+yres/2][x+xres/2]){
                        buffer[y+yres/2][x+xres/2]=NDC.z;
                        //perform depth buffering
                        
                        //Phong Shading
                        //NDC2cam
                        
                        Vector4f v_cam;
                        v_cam=NDC2cam(NDC, P);
                        //cam2world
                        Vector4f v_world;
                        v_world=cam2world(v_cam, camera_matrix_P);
                        vertex v_temp(0,0,0,1);
                        v_temp.x=v_world(0);
                        v_temp.y=v_world(1);
                        v_temp.z=v_world(2);
                        v_temp.w=v_world(3);
                        //lighting_Phong;
                        Vector3f c_lighting_Phong;
//                        cout<<vn.x<<" "<<vn.y<<" "<<vn.z<<" "<<vn.w<<endl;
//                        cout<<v_temp.x<<" "<<v_temp.y<<" "<<v_temp.z<<" "<<v_temp.w<<endl;
                        v_temp.x=v_temp.x/v_temp.w;
                        v_temp.y=v_temp.y/v_temp.w;
                        v_temp.z=v_temp.z/v_temp.w;
                        v_temp.w=1;
                        c_lighting_Phong=lighting(v_temp, vn_temp, obj.diffuse, obj.ambient, obj.specular, obj.shininess, ecam, lights);
                        float color[3];
                        color[0]=c_lighting_Phong(0);
                        color[1]=c_lighting_Phong(1);
                        color[2]=c_lighting_Phong(2);
                        fill(x+xres/2,y+yres/2,grid, color);
                        
                    }
                }
            }
        }
    }
}


int min(int a, int b, int c){
    int temp;
    a>b?temp=b:temp=a;
    temp>c?temp=c:temp=temp;
    return temp;
}

int max(int a, int b, int c){
    int temp;
    a>b?temp=a:temp=b;
    temp>c?temp=temp:temp=c;
    return temp;
}

void fill(int x, int y, RGB** grid, float* color){
    grid[y][x].R=(int)(color[0]*255);
    grid[y][x].G=(int)(color[1]*255);
    grid[y][x].B=(int)(color[2]*255);
    //I used to put 256 here, which caused the color to overflow
}








