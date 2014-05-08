/* Simple opengl demo program. 
 *
 * Author: Papoj Thamjaroenporn
 *         Changxi Zheng
 * Spring 2013
 */

#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>

#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "GLScreenCapturer.h"
#include "trackball.h"
#include "shader.h"
#include <iostream>
#include <vector>
#include "Vector3.h"
#include "Vertex3.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "GLSLProgram.h"

using namespace std;

#define BUFFER_LENGTH 64
#define PI 3.1415926536
GLfloat camRotX, camRotY, camPosX, camPosY, camPosZ;
GLfloat angle = 0.0f;
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];

GLuint pickedObj = -1;
char titleString[150];

bool isTeapot1_selected = false;
bool isTeapot2_selected = false;
bool keydown = false;
float direction = 1.0;

// Lights & Materials
GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat position[] = {0.0, 0.3, 0.0, 1.0};
GLfloat mat_diffuse[] = {0.6, 0.6, 0.6, 1.0};
GLfloat mat_specular[] = {0.7, 0.7, 0.7, 1.0};
GLfloat mat_shininess[] = {50.0};

static GLScreenCapturer screenshot("screenshot-%d.ppm");
static GLSLProgram*     shaderProg = NULL;
static GLSLProgram* toon = NULL;
static GLSLProgram* gour = NULL;
static GLSLProgram* blinnp = NULL;
static GLSLProgram* checkbp = NULL;
static GLSLProgram* bonus = NULL;


#define NUM_X_VERTS       140
#define NUM_Z_VERTS       140
#define NUM_VERTS         NUM_X_VERTS*NUM_Z_VERTS
#define VERT_DISTANCE     0.05
#define BODY_SCALE_FACTOR 2.0
#define BODY_SIZE         NUM_X_VERTS*VERT_DISTANCE*BODY_SCALE_FACTOR

#define VERT_WEIGHT       0.0002
GLuint * Indices;
int NumIndices;   //size of the index array
float initialtime = 0.0f;  //note: this need not be the real time
bool  g_bExcitersInUse = true;

Vertex3 * verts;


void initWater()
{
    verts = new Vertex3[NUM_VERTS];
    Indices = new GLuint[(NUM_VERTS - (NUM_X_VERTS + NUM_Z_VERTS - 1))*6]; 
    int count = 0;
    for (int i = 0; i < NUM_X_VERTS; i++) {
        for (int j = 0; j < NUM_Z_VERTS; j++) 
        {
            verts[i+j*NUM_X_VERTS] = Vertex3::Make(VERT_DISTANCE*float(i)*BODY_SCALE_FACTOR, 0.0, 
                VERT_DISTANCE*float(j)*BODY_SCALE_FACTOR, 0, 1, 0, 
                0, false);

            if ((i < NUM_X_VERTS-1) && (j < NUM_Z_VERTS-1))
            {
                int thisVert = i+j*NUM_X_VERTS;
                
                Indices[count++] = thisVert;
                Indices[count++] = thisVert + 1;
                Indices[count++] = thisVert + NUM_X_VERTS + 1;

                Indices[count++] = thisVert;
                Indices[count++] = thisVert + NUM_X_VERTS + 1;
                Indices[count++] = thisVert + NUM_X_VERTS;
            } 

        }
    }

    verts[100+30*NUM_X_VERTS].bIsExciter = true;
    verts[100+30*NUM_X_VERTS].ExciterAmplitude = 0.8f;
    verts[100+30*NUM_X_VERTS].ExciterFrequency = 15.0f;
    verts[30+80*NUM_X_VERTS].bIsExciter = true;
    verts[30+80*NUM_X_VERTS].ExciterAmplitude = 0.3f;
    verts[30+80*NUM_X_VERTS].ExciterFrequency = 40.0f;
    NumIndices = count;
}

void refreshWater(float deltaTime, float time)
{
    for (int i = 0; i < NUM_X_VERTS; i++) 
    {
        for (int j = 0; j < NUM_Z_VERTS; j++) 
        {
            int thisVert = i+j*NUM_X_VERTS;

            if ((verts[thisVert].bIsExciter) && g_bExcitersInUse)
            {
                verts[thisVert].newY = verts[thisVert].ExciterAmplitude*sin(time*verts[thisVert].ExciterFrequency);
            }


            //check, if this oscillator is on an edge (=>end)
            if ((i==0) || (i==NUM_X_VERTS-1) || (j==0) || (j==NUM_Z_VERTS-1))
                ;//TBD: calculating oscillators at the edge (if the end is free)
            else
            {
              //calculate the new speed:
                

                //Change the speed (=accelerate) according to the oscillator's 4 direct neighbors:
                float AvgDifference = verts[thisVert-1].y             //left neighbor
                                     +verts[thisVert+1].y             //right neighbor
                                     +verts[thisVert-NUM_X_VERTS].y  //upper neighbor
                                     +verts[thisVert+NUM_X_VERTS].y  //lower neighbor
                                     -4*verts[thisVert].y;                //subtract the pos of the current osc. 4 times  
                verts[thisVert].UpSpeed += AvgDifference*deltaTime/VERT_WEIGHT;
                verts[thisVert].newY += verts[thisVert].UpSpeed*deltaTime;
            }
        }       
    }

    //copy the new position to y:
    for (int i = 0; i < NUM_X_VERTS; i++) 
    {
        for (int j = 0; j < NUM_Z_VERTS; j++) 
        {
            verts[i+j*NUM_X_VERTS].y =verts[i+j*NUM_X_VERTS].newY;
        }
    }
    //calculate new normal vectors (according to the oscillator's neighbors):
    for (int i = 1; i < NUM_X_VERTS - 1; i++) 
    {
        for (int j = 1; j < NUM_Z_VERTS - 1; j++) 
        {
            Vector3 u,v;
            
            int thisVert = i+j*NUM_X_VERTS;

            int leftVert = thisVert - 1;
            int rightVert = thisVert + 1;
            int upVert = thisVert - NUM_X_VERTS;
            int downVert = thisVert + NUM_X_VERTS;

            u = Vector3::Make(verts[leftVert].x, verts[leftVert].y, verts[leftVert].z) -
                    Vector3::Make(verts[rightVert].x, verts[rightVert].y, verts[rightVert].z);

            v = Vector3::Make(verts[upVert].x, verts[upVert].y, verts[upVert].z) - 
                    Vector3::Make(verts[downVert].x, verts[downVert].y, verts[downVert].z);

            Vector3 normal = Vector3::Norm(Vector3::Cross(&u,&v));

            verts[thisVert].nx = normal.x; verts[thisVert].ny = normal.y; verts[thisVert].nz = normal.z;
        }
    }

}



void initLights(void)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void setupRC()
{
    tbInit(GLUT_RIGHT_BUTTON);
    tbAnimate(GL_TRUE);
    
    // Place Camera
    camRotX = 0.0f;
    camRotY = 0.0f;
    camPosX = 0.0f;
    camPosY = 0.0f;
    camPosZ = -10.5f;
    
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    initLights();
}

void setCamera( void )
{
    glTranslatef(0, 0, camPosZ);
}

void drawFloor()
{
    glPushMatrix();
    glTranslatef(0,-1,0);
    glRotatef(180,1,0,0);
    glTranslatef(-BODY_SIZE / 2.0, 0, -BODY_SIZE / 2.0);
    glDrawElements( GL_TRIANGLES, //mode
                    NumIndices,  //count, ie. how many indices
                    GL_UNSIGNED_INT, //type of the index array
                    Indices);;
    glPopMatrix();
}
void drawSelectableTeapots( void )
{
    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);
    
    GLfloat selectedColor[] = {0, 1, 0, 1};
    GLfloat unselectedColor[] = {1, 0, 0, 1};

    // Initialize the name stack
    glInitNames();
    glPushName(0);
    drawFloor();
    shaderProg->enable();

    //pass the current time and light direction to the shaders
    shaderProg->set_uniform_1f("time", glutGet(GLUT_ELAPSED_TIME));
    shaderProg->set_uniform_3f("lightDir", 1.f, 1.f, 0.5f);
    // Draw two teapots next to each other in z axis

    
    glPushMatrix();
    {
        
        if( isTeapot1_selected )
            glMaterialfv(GL_FRONT, GL_DIFFUSE, selectedColor);
        else
            glMaterialfv(GL_FRONT, GL_DIFFUSE, unselectedColor);
        glLoadName(0);
        glutSolidSphere(2.5, 40, 40);

        if( isTeapot2_selected )
            glMaterialfv(GL_FRONT, GL_DIFFUSE, selectedColor);
        else
            glMaterialfv(GL_FRONT, GL_DIFFUSE, unselectedColor);
        glLoadName(1);
        glTranslatef(0,0,5);
        glutSolidSphere(0.5, 40, 40);
    }
    glPopMatrix();
    shaderProg->disable();
    
    glColor4fv(currentColor);

}

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glPushMatrix();
    {
    
        setCamera();
        tbMatrix();
        
        drawSelectableTeapots();
        
        // Retrieve current matrice before they popped.
        glGetDoublev( GL_MODELVIEW_MATRIX, modelview );        // Retrieve The Modelview Matrix
        glGetDoublev( GL_PROJECTION_MATRIX, projection );    // Retrieve The Projection Matrix
        glGetIntegerv( GL_VIEWPORT, viewport );                // Retrieves The Viewport Values (X, Y, Width, Height)
    }
    glPopMatrix();

    glFlush();
    // End Drawing calls
    glutSwapBuffers();
}

void reshape( int w, int h )
{
    tbReshape(w, h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set the clipping volume
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 1.0f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void keyboard( unsigned char key, int x, int y )
{
    switch(key)
    {
    case 27: // Escape key
        exit(0);
        break;
    case 'a':
        shaderProg = toon;
        glutPostRedisplay();
        break;
    case 's':
        shaderProg = gour; // apply gourand shader
        glutPostRedisplay();
        break;
    case 'd':
        shaderProg = blinnp; // apply blinn phong shader
        glutPostRedisplay();
        break;
    case 'f':
        shaderProg = checkbp; // apply checker blinn phong shader
        glutPostRedisplay();
        break;
    case 'g':
        shaderProg = bonus; // apply plasma shader
        glutPostRedisplay();
        break;
    case ',':
        keydown = true; //rotate left
        direction = -1.0f;
        break;
    case '.':
        keydown = true; //rotate right
        direction = 1.0f;
        break;
    case 'r':
        printf("save current screen\n");
        screenshot.capture();
        break;
    }
}

//Key is released, stop rotating
void keyup(unsigned char key, int x, int y)
{
    keydown = false;
}

void processSelection(int xPos, int yPos)
{
    GLfloat fAspect;
    
    // Space for selection buffer
    static GLuint selectBuff[BUFFER_LENGTH];
    
    // Hit counter and viewport storage
    GLint hits, viewport[4];
    
    // Setup selection buffer
    glSelectBuffer(BUFFER_LENGTH, selectBuff);
    
    // Get the viewport
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Switch to projection and save the matrix
    glMatrixMode(GL_PROJECTION);
    
    glPushMatrix();
    {
        // Change render mode
        glRenderMode(GL_SELECT);
        
        // Establish new clipping volume to be unit cube around
        // mouse cursor point (xPos, yPos) and extending two pixels
        // in the vertical and horizontal direction
        glLoadIdentity();
        gluPickMatrix(xPos, viewport[3] - yPos + viewport[1], 0.1,0.1, viewport);
        
        // Apply perspective matrix 
        fAspect = (float)viewport[2] / (float)viewport[3];
        gluPerspective(45.0f, fAspect, 1.0, 425.0);
        
        
        // Render only those needed for selection
        glPushMatrix();    
        {
            setCamera();
            tbMatrixForSelection();
            
            drawSelectableTeapots();
        }
        glPopMatrix();
        
        
        // Collect the hits
        hits = glRenderMode(GL_RENDER);
        
        isTeapot1_selected = false;
        isTeapot2_selected = false;
        
        // If hit(s) occurred, display the info.
        if(hits != 0)
        {
        
            // Save current picked object.
            // Take only the nearest selection
            pickedObj = selectBuff[3];
            
            sprintf (titleString, "You clicked on %d", pickedObj);
            glutSetWindowTitle(titleString);
            
            if (pickedObj == 0) {
                isTeapot1_selected = true;
            }
            
            if (pickedObj == 1) {
                isTeapot2_selected = true;
            }
            
        }
        else
            glutSetWindowTitle("Nothing was clicked on!");
        
        
        // Restore the projection matrix
        glMatrixMode(GL_PROJECTION);
    }
    glPopMatrix();
    
    // Go back to modelview for normal rendering
    glMatrixMode(GL_MODELVIEW);
    
    glutPostRedisplay();
}

void mouse( int button, int state, int x, int y)
{
    tbMouse(button, state, x, y);
    
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        processSelection(x, y);
    
    if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        pickedObj = -1;
        glutPostRedisplay();
    }
}

void motion(int x, int y)
{
    tbMotion(x, y);
    
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
    
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    glutPostRedisplay();

}

static void setupShaders()
{
    printf("MSG: Initialize GLSL Shaders ...\n");

    glewInit();
    if ( !glewIsSupported("GL_VERSION_2_0 GL_ARB_multitexture GL_EXT_framebuffer_object") ) 
    {
        fprintf(stderr, "Required OpenGL extensions missing\n");
        exit(2);
    }
    toon = new GLSLProgram(toonVS, toonFS);
    gour = new GLSLProgram(gourVS, gourFS);
    blinnp = new GLSLProgram(blinnpVS, blinnpFS);
    checkbp = new GLSLProgram(checkbpVS, checkbpFS);
    bonus = new GLSLProgram(bonusVS, bonusFS);
    shaderProg = toon;
}

void Idle(void)
{
    float dtime = 0.004f;  //if you want to be exact, you would have to replace this by the real time passed since the last frame (and probably divide it by a certain number)
    initialtime += dtime;

    if (initialtime > 1.7f)
    {
        g_bExcitersInUse = false;  //stop the exciters
    }
//ENABLE THE FOLLOWING LINES FOR A RAIN EFFECT
    int randomNumber = rand();
    if (randomNumber < NUM_VERTS)
    {
        verts[randomNumber].y = -0.05;
    }
    if(keydown) {
        glTranslatef(0, 0, camPosZ);
        glRotatef(1.0 * direction, 0.0, 1.0, 0.0);
        glTranslatef(0, 0, -camPosZ);
    }
    


    refreshWater(dtime,initialtime);
    display();
}

int main (int argc, char *argv[])
{
    int win_width = 960;
    int win_height = 540;

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( win_width, win_height );

    glutCreateWindow( "Opengl demo" );
    setupShaders();
    setupRC();

    initWater();
    //Enable the vertex array functionality:
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(    3,   //3 components per vertex (x,y,z)
                        GL_FLOAT,
                        sizeof(Vertex3),
                        verts);
    glNormalPointer(    GL_FLOAT,
                        sizeof(Vertex3),
                        &verts[0].nx);  //Pointer to the first color*/
    glPointSize(2.0);
    glClearColor(0.0,0.0,0.0,0.0);
    
    //Switch on solid rendering:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutKeyboardUpFunc( keyup );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );
    glutIdleFunc(Idle);

    glutMainLoop();
    delete shaderProg;
}
