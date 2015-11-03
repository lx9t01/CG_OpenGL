
#define _USE_MATH_DEFINES //M_PI for 3.14, deg->rad

#include "readobj.h"

using namespace std;
using namespace Eigen;


///////////////////////////////////////////////////////////////////////////////////////////////////


void init(void);
void reshape(int width, int height);
void display(void);

void init_lights();
void set_lights();
void draw_objects();

void mouse_pressed(int button, int state, int x, int y);
void mouse_moved(int x, int y);
void key_pressed(unsigned char key, int x, int y);

void parse(char* filename);
GLfloat* getCurrentRotaion();

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

float cam_position[3]; //xyz, global varible
float cam_orientation_axis[3];
float cam_orientation_angle = 0; // in deg
float near_param = 1, far_param = 20,
      left_param = -0.5, right_param = 0.5,
      top_param = 0.5, bottom_param = -0.5;
float sita;
float u[3];
vector<Point_Light> lights;
vector<GraphObj> graph;
int mouse_x, mouse_y;
float mouse_scale_x, mouse_scale_y;
Matrix4f current_rotation=Matrix4f::Identity(4,4);
Matrix4f last_rotation=Matrix4f::Identity(4,4);
GLfloat temp2[16];
const float step_size = 0.2;
const float x_view_step = 90.0, y_view_step = 90.0;
float x_view_angle = 0, y_view_angle = 0;
bool is_pressed = false;
bool wireframe_mode = false;
int xres, yres;

///////////////////////////////////////////////////////////////////////////////////////////////////

void init(void)
{
    glShadeModel(GL_SMOOTH);
    //glShadeModel(GL_FLAT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); //backface culling
    glEnable(GL_DEPTH_TEST); //depth buffering
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glMatrixMode(GL_PROJECTION); // projection mode
    glLoadIdentity(); //projection matrix=I
    glFrustum(left_param, right_param,
              bottom_param, top_param,
              near_param, far_param); // camera space F
     //P = P * F, P=I=>F
    glMatrixMode(GL_MODELVIEW); //modelview mode
    init_lights();
}

void reshape(int width, int height)
{
    height = (height == 0) ? 1 : height;
    width = (width == 0) ? 1 : width;
    glViewport(0, 0, width, height);
    mouse_scale_x = (float) (right_param - left_param) / (float) width;
    mouse_scale_y = (float) (top_param - bottom_param) / (float) height;
    glutPostRedisplay();//re-render
}

void display(void) //world & camera space
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // window black, depth buffer value high
    glLoadIdentity();
    glRotatef(y_view_angle, 1, 0, 0); //camera rotate first, ini 0
    glRotatef(x_view_angle, 0, 1, 0);
    glRotatef(-cam_orientation_angle,
              cam_orientation_axis[0], cam_orientation_axis[1], cam_orientation_axis[2]);
    // inv rotation matrix
    glTranslatef(-cam_position[0], -cam_position[1], -cam_position[2]);
    // inv trans
    set_lights(); // init lights
    draw_objects();// draw objs
    //double buffering
    glutSwapBuffers(); //swap double buffers, active/off-screen
}


void init_lights()
{
    glEnable(GL_LIGHTING); //Phong to every vertex
    int num_lights = lights.size();
    for(int i = 0; i < num_lights; ++i)
    {
        int light_id = GL_LIGHT0 + i; //GL_LIGHT0 - 7
        glEnable(light_id);
        glLightfv(light_id, GL_AMBIENT, lights[i].color);
        glLightfv(light_id, GL_DIFFUSE, lights[i].color);
        glLightfv(light_id, GL_SPECULAR, lights[i].color);
        glLightf(light_id, GL_QUADRATIC_ATTENUATION, lights[i].attenuation_k);
    }
}

void set_lights() //position lights in camera space
{
    int num_lights = lights.size();
    
    for(int i = 0; i < num_lights; ++i)
    {
        int light_id = GL_LIGHT0 + i;
        glLightfv(light_id, GL_POSITION, lights[i].position);
    }
}

void draw_objects()
{
    int num_objects = graph.size();
    for(int i = 0; i < num_objects; i++)
    {
        glPushMatrix(); //keep original matrix in stack
        {
            int num_transform_sets = graph[i].transform_sets.size();
            for(int j = num_transform_sets-1; j >=0; j--)
            {
                glTranslatef(graph[i].transform_sets[j].translation[0],
                             graph[i].transform_sets[j].translation[1],
                             graph[i].transform_sets[j].translation[2]);
                glRotatef(graph[i].transform_sets[j].rotation_angle,
                          graph[i].transform_sets[j].rotation[0],
                          graph[i].transform_sets[j].rotation[1],
                          graph[i].transform_sets[j].rotation[2]);
//                cout<<graph[i].transform_sets[j].rotation_angle<<" "<<graph[i].transform_sets[j].rotation[0]<<" "<<graph[i].transform_sets[j].rotation[1]<<" "<<graph[i].transform_sets[j].rotation[2]<<endl;
                glMultMatrixf(getCurrentRotaion());
                glScalef(graph[i].transform_sets[j].scaling[0],
                         graph[i].transform_sets[j].scaling[1],
                         graph[i].transform_sets[j].scaling[2]);
                //IF THE TRANS DOES NOT CONCERN SCALING, DONT FORGET TO SET SCALE PARAMETER 1111
                
//                cout<<"tr: "<<graph[i].transform_sets[j].translation[0]<<" "<<graph[i].transform_sets[j].translation[1]<<" "<<graph[i].transform_sets[j].translation[2]<<endl;
//                cout<<"ro: "<<graph[i].transform_sets[j].rotation_angle<<" "<<graph[i].transform_sets[j].rotation[0]<<" "<<graph[i].transform_sets[j].rotation[1]<<" "<<graph[i].transform_sets[j].rotation[2]<<endl;
//                cout<<"sc: "<<graph[i].transform_sets[j].scaling[0]<<" "<<graph[i].transform_sets[j].scaling[1]<<" "<<graph[i].transform_sets[j].scaling[2]<<endl;
//                cout<<endl;
            }
            glMaterialfv(GL_FRONT, GL_AMBIENT, graph[i].ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, graph[i].diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, graph[i].specular);
            glMaterialf(GL_FRONT, GL_SHININESS, graph[i].shininess);
            vector<face> face_temp=graph[i].getFace();
            vector<vertex> vertex_temp=graph[i].getVertex();
            vector<vertexn> vertexN_temp=graph[i].getVertexN();

            int num_faces= face_temp.size();
            for (int j=0; j<num_faces; j++){
                vertex v_a(0,0,0);
                vertex v_b(0,0,0);
                vertex v_c(0,0,0);
                vertexn vn_a(0,0,0);
                vertexn vn_b(0,0,0);
                vertexn vn_c(0,0,0);

                v_a.x=vertex_temp[face_temp[j].a-1].x;
                v_a.y=vertex_temp[face_temp[j].a-1].y;
                v_a.z=vertex_temp[face_temp[j].a-1].z;
                v_b.x=vertex_temp[face_temp[j].b-1].x;
                v_b.y=vertex_temp[face_temp[j].b-1].y;
                v_b.z=vertex_temp[face_temp[j].b-1].z;
                v_c.x=vertex_temp[face_temp[j].c-1].x;
                v_c.y=vertex_temp[face_temp[j].c-1].y;
                v_c.z=vertex_temp[face_temp[j].c-1].z;
                graph[i].vertex_buffer.push_back(v_a);
                graph[i].vertex_buffer.push_back(v_b);
                graph[i].vertex_buffer.push_back(v_c);

                vn_a.x=vertexN_temp[face_temp[j].an-1].x;
                vn_a.y=vertexN_temp[face_temp[j].an-1].y;
                vn_a.z=vertexN_temp[face_temp[j].an-1].z;
                vn_b.x=vertexN_temp[face_temp[j].bn-1].x;
                vn_b.y=vertexN_temp[face_temp[j].bn-1].y;
                vn_b.z=vertexN_temp[face_temp[j].bn-1].z;
                vn_c.x=vertexN_temp[face_temp[j].cn-1].x;
                vn_c.y=vertexN_temp[face_temp[j].cn-1].y;
                vn_c.z=vertexN_temp[face_temp[j].cn-1].z;
                graph[i].vertexN_buffer.push_back(vn_a);
                graph[i].vertexN_buffer.push_back(vn_b);
                graph[i].vertexN_buffer.push_back(vn_c);
            }

            glVertexPointer(3, GL_FLOAT, 0, &graph[i].vertex_buffer[0]); //triangle

            glNormalPointer(GL_FLOAT, 0, &graph[i].vertexN_buffer[0]);
            
            int buffer_size = graph[i].vertex_buffer.size();
            
            if(!wireframe_mode)
                glDrawArrays(GL_TRIANGLES, 0, buffer_size); //first index, num objs
            else
                for(int j = 0; j < buffer_size; j += 3)
                    glDrawArrays(GL_LINE_LOOP, j, 3);
        }
        glPopMatrix();
    }
    
    
//    glPushMatrix();
//    {
//        glTranslatef(0, -103, 0);
//        glutSolidSphere(100, 100, 100);
//    }
//    glPopMatrix();
    
}

void mouse_pressed(int button, int state, int x, int y)
{
    /* If the left-mouse button was clicked down, then...
     */
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) //GLUT_DOWN/GLUT_UP
    {
        /* Store the mouse position in our global variables.
         */
        mouse_x = x;
        mouse_y = y;
        
        /* Since the mouse is being pressed down, we set our 'is_pressed"
         * boolean indicator to true.
         */
        is_pressed = true;
    }
    /* If the left-mouse button was released up, then...
     */
    else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        /* Mouse is no longer being pressed, so set our indicator to false.
         */
        last_rotation=current_rotation*last_rotation;
        current_rotation=Matrix4f::Identity(4,4);
        is_pressed = false;
    }
}

void mouse_moved(int x, int y)
{
    /* If the left-mouse button is being clicked down...
     */
    if(is_pressed)
    {
        
        float mouse_x_temp, mouse_y_temp, mouse_z_temp;
        
        mouse_x_temp=2*mouse_x*mouse_scale_x-1;
        mouse_y_temp=-(2*mouse_y*mouse_scale_y-1);
//        cout<<"mx"<<mouse_x_temp<<" "<<mouse_scale_x<<endl;
//        cout<<"my"<<mouse_y_temp<<" "<<mouse_scale_y<<endl;

        float x_temp, y_temp, z_temp;
        x_temp=2*x*mouse_scale_x-1;
        y_temp=-(2*y*mouse_scale_y-1);
        


        if(mouse_x_temp*mouse_x_temp+mouse_y_temp*mouse_y_temp<=1){
            mouse_z_temp=sqrt(1-mouse_x_temp*mouse_x_temp-mouse_y_temp*mouse_y_temp);
//            cout<<"mz"<<mouse_z_temp<<endl;
            z_temp=sqrt(1-x_temp*x_temp-y_temp*y_temp);
//              cout<<"x"<<x_temp<<" "<<mouse_scale_x<<endl;
//              cout<<"y"<<y_temp<<" "<<mouse_scale_y<<endl;
            Vector3f p1, p2;
            Vector3f vu;
            p1<<mouse_x_temp, mouse_y_temp, mouse_z_temp;
            p2<<x_temp, y_temp, z_temp;
//            cout<<p1<<endl;
//            cout<<p2<<endl;
            float temp=p1.dot(p2)/(p1.norm()*p2.norm());
            if (temp<1) {
                sita=acos(temp); // in rad
            }else{
                sita=0;
            }
            vu=p1.cross(p2);
            vu=vu/vu.norm();
            u[0]=vu(0);
            u[1]=vu(1);
            u[2]=vu(2);
        }else{
            u[0]=0;
            u[1]=0;
            u[2]=1;// z axis
            sita=-(atan(mouse_y_temp/mouse_x_temp)-atan(y_temp/x_temp)); //in rad
            
        }
        float qs, qx, qy, qz;
        qs=cos(sita/2);
        qx=u[0]*sin(sita/2);
        qy=u[1]*sin(sita/2);
        qz=u[2]*sin(sita/2);
        
        current_rotation(0,0)=1-2*qy*qy-2*qz*qz;
        current_rotation(0,1)=2*(qx*qy-qz*qs);
        current_rotation(0,2)=2*(qx*qz+qy*qs);
        current_rotation(0,3)=0;
        current_rotation(1,0)=2*(qx*qy+qz*qs);
        current_rotation(1,1)=1-2*qx*qx-2*qz*qz;
        current_rotation(1,2)=2*(qy*qz-qx*qs);
        current_rotation(1,3)=0;
        current_rotation(2,0)=2*(qx*qz-qy*qs);
        current_rotation(2,1)=2*(qy*qz+qx*qs);
        current_rotation(2,2)=1-2*qx*qx-2*qy*qy;
        current_rotation(2,3)=0;
        current_rotation(3,0)=0;
        current_rotation(3,1)=0;
        current_rotation(3,2)=0;
        current_rotation(3,3)=1;
        
//        x_view_angle += ((float) x - (float) mouse_x) * mouse_scale_x * x_view_step;
//
//        float temp_y_view_angle = y_view_angle +
//                                  ((float) y - (float) mouse_y) * mouse_scale_y * y_view_step;
//        y_view_angle = (temp_y_view_angle > 90 || temp_y_view_angle < -90) ?
//                       y_view_angle : temp_y_view_angle;
        
//        /* We update our 'mouse_x' and 'mouse_y' variables so that if the user moves
//         * the mouse again without releasing it, then the distance we compute on the
//         * next call to the 'mouse_moved' function will be from this current mouse
//         * position.
//         */
//
//        mouse_x = x;
//        mouse_y = y;
        //DEFINITELY GET RID OF THESE TWO LINES!!!! THEY ARE EVIL.
        
        /* Tell OpenGL that it needs to re-render our scene with the new camera
         * angles.
         */
        glutPostRedisplay();
    }
}

GLfloat* getCurrentRotaion(){
    Matrix4f temp1;
    temp1=current_rotation*last_rotation;
    cout<<"current\n"<<current_rotation<<endl;
    cout<<"last\n"<<last_rotation<<endl;
//    cout<<current_rotation<<endl;
//    cout<<last_rotation<<endl;
//    cout<<temp1<<endl;

    temp2[0]=temp1(0,0);
    temp2[1]=temp1(1,0);
    temp2[2]=temp1(2,0);
    temp2[3]=temp1(3,0);
    temp2[4]=temp1(0,1);
    temp2[5]=temp1(1,1);
    temp2[6]=temp1(2,1);
    temp2[7]=temp1(3,1);
    temp2[8]=temp1(0,2);
    temp2[9]=temp1(1,2);
    temp2[10]=temp1(2,2);
    temp2[11]=temp1(3,2);
    temp2[12]=temp1(0,3);
    temp2[13]=temp1(1,3);
    temp2[14]=temp1(2,3);
    temp2[15]=temp1(3,3);
//    for (int i=0;i<16;i++){
//        cout<<temp2[i]<<" ";
//    }
//    cout<<endl;

    return temp2;
}

float deg2rad(float angle)
{
    return angle * M_PI / 180.0;
}

void key_pressed(unsigned char key, int x, int y)
{

    if(key == 'q')
    {
        exit(0);
    }

    else if(key == 't')
    {
        wireframe_mode = !wireframe_mode;

        glutPostRedisplay();
    }
    else
    {
        float x_view_rad = deg2rad(x_view_angle);

        if(key == 'w')
        {
            cam_position[0] += step_size * sin(x_view_rad);
            cam_position[2] -= step_size * cos(x_view_rad);
            glutPostRedisplay();
        }
        /* 'a' for step left
         */
        else if(key == 'a')
        {
            cam_position[0] -= step_size * cos(x_view_rad);
            cam_position[2] -= step_size * sin(x_view_rad);
            glutPostRedisplay();
        }
        /* 's' for step backward
         */
        else if(key == 's')
        {
            cam_position[0] -= step_size * sin(x_view_rad);
            cam_position[2] += step_size * cos(x_view_rad);
            glutPostRedisplay();
        }
        /* 'd' for step right
         */
        else if(key == 'd')
        {
            cam_position[0] += step_size * cos(x_view_rad);
            cam_position[2] += step_size * sin(x_view_rad);
            glutPostRedisplay();
        }
    }
}


void parse(char* filename){

    ifstream graphfile(filename);

    string line;
    string objname, objfilename;
    unsigned int flag = 0;
    unsigned int number = 0;
    string stringtemp, command;
    
    int numtemp=0;
    
    if (graphfile.is_open()){
        while(getline(graphfile,line)){//read each line
            //read camera
            if (line.size() >0 && flag ==0){
                getline(graphfile, line);//camera:
                stringstream ss(line);
                getline(ss, command, ' ');
                for (int i=0;i<3;i++){
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>cam_position[i];
                }
                
                getline(graphfile, line);
                stringstream ss1(line);
                getline(ss1, command, ' ');
                float cam_ori[3];
                for (int i=0;i<3;i++){
                    getline(ss1, stringtemp, ' ');
                    stringstream(stringtemp)>>cam_ori[i];
                }
                float cam_norm;
                cam_norm=sqrt(cam_ori[0]*cam_ori[0]+cam_ori[1]*cam_ori[1]+cam_ori[2]*cam_ori[2]);
                for (int i=0;i<3;i++){
                    cam_orientation_axis[i]=cam_ori[i]/cam_norm;
                }
                getline(ss1, stringtemp, ' ');
                stringstream(stringtemp)>>cam_orientation_angle;
                cam_orientation_angle=cam_orientation_angle*180/M_PI;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>near_param;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>far_param;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>left_param;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>right_param;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>top_param;
                getline(graphfile, command, ' ');
                getline(graphfile, stringtemp);
                stringstream(stringtemp)>>bottom_param;
            }else if (line.size()==0 && flag==0){//read in the obj files
                flag=1;
                getline(graphfile,line);
            }
            
            // read in lights
            if(line.size()!=0 && flag==1){

                Point_Light light1;

                string lighttemp;
                stringstream ss_l(line);
                getline(ss_l, stringtemp, ' ');
                getline(ss_l, stringtemp, ',');// , as separation. xyz
                stringstream ss_l1(stringtemp);
                for (int i=0;i<3;i++){
                    getline(ss_l1, lighttemp, ' ');
                    stringstream(lighttemp)>>light1.position[i];
                }
                light1.position[3]=1;
                getline(ss_l, stringtemp, ',');// , as separation. rgb
                stringstream ss_l2(stringtemp);
                getline(ss_l2, lighttemp, ' ');// avoid one more ' '
                for (int i=0;i<3;i++){
                    getline(ss_l2, lighttemp, ' ');
                    stringstream(lighttemp)>>light1.color[i];
                }
                getline(ss_l, stringtemp);// , as separation. k
                stringstream ss_l3(stringtemp);
                ss_l3>>light1.attenuation_k;
                
                lights.push_back(light1);

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
                    Transforms trans_temp;
                    for (int i=0;i<3;i++){
                        getline(ss, stringtemp,' ');
                        stringstream(stringtemp)>>trans_temp.translation[i];
                    }
                    trans_temp.scaling[0]=1;
                    trans_temp.scaling[1]=1;
                    trans_temp.scaling[2]=1;
                    graph[numtemp].transform_sets.push_back(trans_temp);
                    
                }else if (stringtemp=="r"){
                    Transforms trans_temp;
                    for (int i=0;i<3;i++){
                        getline(ss, stringtemp,' ');
                        stringstream(stringtemp)>>trans_temp.rotation[i];
                    }
                    float norm = sqrt(trans_temp.rotation[0]*trans_temp.rotation[0]+trans_temp.rotation[1]*trans_temp.rotation[1]+trans_temp.rotation[2]*trans_temp.rotation[2]);
                    trans_temp.rotation[0]=trans_temp.rotation[0]/norm;
                    trans_temp.rotation[1]=trans_temp.rotation[1]/norm;
                    trans_temp.rotation[2]=trans_temp.rotation[2]/norm;
                    getline(ss, stringtemp, ' ');
                    stringstream(stringtemp)>>trans_temp.rotation_angle;
                    trans_temp.scaling[0]=1;
                    trans_temp.scaling[1]=1;
                    trans_temp.scaling[2]=1;
                    trans_temp.rotation_angle=trans_temp.rotation_angle*180/M_PI;
                    graph[numtemp].transform_sets.push_back(trans_temp);

                }else if (stringtemp=="s"){
                    Transforms trans_temp;
                    for (int i=0;i<3;i++){
                        getline(ss, stringtemp, ' ');
                        stringstream(stringtemp)>>trans_temp.scaling[i];
                    }
                    graph[numtemp].transform_sets.push_back(trans_temp);
                  
                }else{
                    cout << "No such transform!\n";
                }//if flag=2 && not empty line
            }
        }
        //end of read
        //end of output obj vertex
        
    }else{
        cout <<"Graph file not open.\n";
        return;
    }//if open graph file

}
/* The 'main' function:
 *
 * This function is short, but is basically where everything comes together.
 */
int main(int argc, char* argv[])
{
    xres = atoi(argv[2]);
    yres = atoi(argv[3]);

    parse(argv[1]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(xres, yres);
    glutInitWindowPosition(0, 0); //top-left
    glutCreateWindow("glRENDER");
    
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse_pressed);
    glutMotionFunc(mouse_moved);
    glutKeyboardFunc(key_pressed);
    glutMainLoop();
}
