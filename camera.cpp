//
//  camera.cpp
//  hw1
//
//  Created by 徐海翔 on 15/10/11.
//  Copyright © 2015年 徐海翔. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "readobj.h"

Matrix4f camera::setInvC(float cam_p[3], float cam_o[4]){
    Matrix4f ct, cr, C;
    
    ct<<1,0,0,cam_p[0],
    0,1,0,cam_p[1],
    0,0,1,cam_p[2],
    0,0,0,1;
    //normalize the orientation of camera!!!
    float norm;
    norm=sqrt(cam_o[0]*cam_o[0]+cam_o[1]*cam_o[1]+cam_o[2]*cam_o[2]);
    cam_o[0]=cam_o[0]/norm;
    cam_o[1]=cam_o[1]/norm;
    cam_o[2]=cam_o[2]/norm;
    
    cr=Matrix4f::Zero(4,4);
    cr(0,0)=cam_o[0]*cam_o[0]+(1-cam_o[0]*cam_o[0])*cos(cam_o[3]);
    cr(1,1)=cam_o[1]*cam_o[1]+(1-cam_o[1]*cam_o[1])*cos(cam_o[3]);
    cr(2,2)=cam_o[2]*cam_o[2]+(1-cam_o[2]*cam_o[2])*cos(cam_o[3]);
    cr(3,3)=1;
    cr(0,1)=cam_o[0]*cam_o[1]*(1-cos(cam_o[3]))-cam_o[2]*sin(cam_o[3]);
    cr(1,0)=cam_o[0]*cam_o[1]*(1-cos(cam_o[3]))+cam_o[2]*sin(cam_o[3]);
    cr(0,2)=cam_o[0]*cam_o[2]*(1-cos(cam_o[3]))+cam_o[1]*sin(cam_o[3]);
    cr(2,0)=cam_o[0]*cam_o[2]*(1-cos(cam_o[3]))-cam_o[1]*sin(cam_o[3]);
    cr(1,2)=cam_o[1]*cam_o[2]*(1-cos(cam_o[3]))-cam_o[0]*sin(cam_o[3]);
    cr(2,1)=cam_o[1]*cam_o[2]*(1-cos(cam_o[3]))+cam_o[0]*sin(cam_o[3]);
    //cout<<"cr: "<<cr<<"\n";
    C=ct*cr;
    inv_C=C.inverse();
    return inv_C;
}

Matrix4f camera::getInvC(){
    return inv_C;
}