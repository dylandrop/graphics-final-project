/* Simple opengl demo program. 
 *
 * Author: Papoj Thamjaroenporn
 *         Changxi Zheng
 * Spring 2013
 */

#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#if defined(_WIN32)
#   include <GL/wglew.h>
#endif

#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "GLScreenCapturer.h"
#include "trackball.h"
#include "shader.h"
#include <vector>
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


#define NUM_X_OSCILLATORS       150
#define NUM_Z_OSCILLATORS       150
#define NUM_OSCILLATORS         NUM_X_OSCILLATORS*NUM_Z_OSCILLATORS
#define OSCILLATOR_DISTANCE     0.05

#define OSCILLATOR_WEIGHT       0.0002
int NumOscillators;  //size of the vertex array
vector <GLuint> IndexVect;  //we first put the indices into this vector, then copy them to the array below
GLuint * Indices;
int NumIndices;   //size of the index array
float g_timePassedSinceStart = 0.0f;  //note: this need not be the real time
bool  g_bExcitersInUse = true;


///////////////////////////////////////////////////
//Required for calculating the normals:
struct SOscillator
{
    GLfloat x,y,z;
    GLfloat nx,ny,nz;  //normal vector
    GLfloat UpSpeed;
    GLfloat newY;
    bool bIsExciter;
    //only in use, if bIsExciter is true:
    float ExciterAmplitude;  
    float ExciterFrequency;
};
SOscillator * Oscillators;

struct SF3dVector  //Float 3d-vect, normally used
{
    GLfloat x,y,z;
};
struct SF2dVector
{
    GLfloat x,y;
};

SF3dVector F3dVector ( GLfloat x, GLfloat y, GLfloat z )
{
    SF3dVector tmp;
    tmp.x = x;
    tmp.y = y;
    tmp.z = z;
    return tmp;
}
SF3dVector AddF3dVectors (SF3dVector* u, SF3dVector* v)
{
    SF3dVector result;
    result.x = u->x + v->x;
    result.y = u->y + v->y;
    result.z = u->z + v->z;
    return result;
}
void AddF3dVectorToVector ( SF3dVector * Dst, SF3dVector * V2)
{
    Dst->x += V2->x;
    Dst->y += V2->y;
    Dst->z += V2->z;
}
GLfloat GetF3dVectorLength( SF3dVector * v)
{
    return (GLfloat)(sqrt(v->x*v->x+v->y*v->y+v->z*v->z));  
}
SF3dVector CrossProduct (SF3dVector * u, SF3dVector * v)
{
    SF3dVector resVector;
    resVector.x = u->y*v->z - u->z*v->y;
    resVector.y = u->z*v->x - u->x*v->z;
    resVector.z = u->x*v->y - u->y*v->x;
    return resVector;
}
SF3dVector Normalize3dVector( SF3dVector v)
{
    SF3dVector res;
    float l = GetF3dVectorLength(&v);
    if (l == 0.0f) return F3dVector(0.0f,0.0f,0.0f);
    res.x = v.x / l;
    res.y = v.y / l;
    res.z = v.z / l;
    return res;
}
SF3dVector operator+ (SF3dVector v, SF3dVector u)
{
    SF3dVector res;
    res.x = v.x+u.x;
    res.y = v.y+u.y;
    res.z = v.z+u.z;
    return res;
}
SF3dVector operator- (SF3dVector v, SF3dVector u)
{
    SF3dVector res;
    res.x = v.x-u.x;
    res.y = v.y-u.y;
    res.z = v.z-u.z;
    return res;
}
///////////////////////////////////////////////////


void CreatePool()
{
    NumOscillators = NUM_OSCILLATORS;
    Oscillators = new SOscillator[NumOscillators];
    IndexVect.clear();  //to be sure it is empty
    for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++) 
        for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++) 
        {
            Oscillators[xc+zc*NUM_X_OSCILLATORS].x = OSCILLATOR_DISTANCE*float(xc);
            Oscillators[xc+zc*NUM_X_OSCILLATORS].y = 0.0f;
            Oscillators[xc+zc*NUM_X_OSCILLATORS].z = OSCILLATOR_DISTANCE*float(zc);

            Oscillators[xc+zc*NUM_X_OSCILLATORS].nx = 0.0f;
            Oscillators[xc+zc*NUM_X_OSCILLATORS].ny = 1.0f;
            Oscillators[xc+zc*NUM_X_OSCILLATORS].nz = 0.0f;

            Oscillators[xc+zc*NUM_X_OSCILLATORS].UpSpeed = 0;
            Oscillators[xc+zc*NUM_X_OSCILLATORS].bIsExciter = false;

            //create two triangles:
            if ((xc < NUM_X_OSCILLATORS-1) && (zc < NUM_Z_OSCILLATORS-1))
            {
                IndexVect.push_back(xc+zc*NUM_X_OSCILLATORS);
                IndexVect.push_back((xc+1)+zc*NUM_X_OSCILLATORS);
                IndexVect.push_back((xc+1)+(zc+1)*NUM_X_OSCILLATORS);

                IndexVect.push_back(xc+zc*NUM_X_OSCILLATORS);
                IndexVect.push_back((xc+1)+(zc+1)*NUM_X_OSCILLATORS);
                IndexVect.push_back(xc+(zc+1)*NUM_X_OSCILLATORS);
            }

        }

    //copy the indices:
    Indices = new GLuint[IndexVect.size()];  //allocate the required memory
    for (int i = 0; i < IndexVect.size(); i++)
    {
        Indices[i] = IndexVect[i];
    }

    Oscillators[100+30*NUM_X_OSCILLATORS].bIsExciter = true;
    Oscillators[100+30*NUM_X_OSCILLATORS].ExciterAmplitude = 0.8f;
    Oscillators[100+30*NUM_X_OSCILLATORS].ExciterFrequency = 15.0f;
    Oscillators[30+80*NUM_X_OSCILLATORS].bIsExciter = true;
    Oscillators[30+80*NUM_X_OSCILLATORS].ExciterAmplitude = 0.3f;
    Oscillators[30+80*NUM_X_OSCILLATORS].ExciterFrequency = 40.0f;
    NumIndices = IndexVect.size();
    IndexVect.clear();  //no longer needed, takes only memory
}

void UpdateScene(bool bEndIsFree, float deltaTime, float time)
{
//********
// Here we do the physical calculations: 
// The oscillators are moved according to their neighbors.
// The parameter bEndIsFree indicates, whether the oscillators in the edges can move or not.
// The new position may be assigned not before all calculations are done!

// PLEASE NOTE: THESE ARE APPROXIMATIONS AND I KNOW THIS! (but is looks good, doesn't it?)

    //if we use two loops, it is a bit easier to understand what I do here.
    for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++) 
    {
        for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++) 
        {
            int ArrayPos = xc+zc*NUM_X_OSCILLATORS;

            //check, if oscillator is an exciter (these are not affected by other oscillators)
            if ((Oscillators[ArrayPos].bIsExciter) && g_bExcitersInUse)
            {
                Oscillators[ArrayPos].newY = Oscillators[ArrayPos].ExciterAmplitude*sin(time*Oscillators[ArrayPos].ExciterFrequency);
            }


            //check, if this oscillator is on an edge (=>end)
            if ((xc==0) || (xc==NUM_X_OSCILLATORS-1) || (zc==0) || (zc==NUM_Z_OSCILLATORS-1))
                ;//TBD: calculating oscillators at the edge (if the end is free)
            else
            {
              //calculate the new speed:
                

                //Change the speed (=accelerate) according to the oscillator's 4 direct neighbors:
                float AvgDifference = Oscillators[ArrayPos-1].y             //left neighbor
                                     +Oscillators[ArrayPos+1].y             //right neighbor
                                     +Oscillators[ArrayPos-NUM_X_OSCILLATORS].y  //upper neighbor
                                     +Oscillators[ArrayPos+NUM_X_OSCILLATORS].y  //lower neighbor
                                     -4*Oscillators[ArrayPos].y;                //subtract the pos of the current osc. 4 times  
                Oscillators[ArrayPos].UpSpeed += AvgDifference*deltaTime/OSCILLATOR_WEIGHT;

              //calculate the new position, but do not yet store it in "y" (this would affect the calculation of the other osc.s)
                Oscillators[ArrayPos].newY += Oscillators[ArrayPos].UpSpeed*deltaTime;
              
                
                
            }
        }       
    }

    //copy the new position to y:
    for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++) 
    {
        for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++) 
        {
            Oscillators[xc+zc*NUM_X_OSCILLATORS].y =Oscillators[xc+zc*NUM_X_OSCILLATORS].newY;
        }
    }
    //calculate new normal vectors (according to the oscillator's neighbors):
    for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++) 
    {
        for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++) 
        {
            ///
            //Calculating the normal:
            //Take the direction vectors 1.) from the left to the right neighbor 
            // and 2.) from the upper to the lower neighbor.
            //The vector orthogonal to these 

            SF3dVector u,v,p1,p2;   //u and v are direction vectors. p1 / p2: temporary used (storing the points)

            if (xc > 0) p1 = F3dVector(Oscillators[xc-1+zc*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc-1+zc*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc-1+zc*NUM_X_OSCILLATORS].z);
            else
                        p1 = F3dVector(Oscillators[xc+zc*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].z); 
            if (xc < NUM_X_OSCILLATORS-1) 
                        p2 = F3dVector(Oscillators[xc+1+zc*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+1+zc*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+1+zc*NUM_X_OSCILLATORS].z);
            else
                        p2 = F3dVector(Oscillators[xc+zc*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].z); 
            u = p2-p1; //vector from the left neighbor to the right neighbor
            if (zc > 0) p1 = F3dVector(Oscillators[xc+(zc-1)*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+(zc-1)*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+(zc-1)*NUM_X_OSCILLATORS].z);
            else
                        p1 = F3dVector(Oscillators[xc+zc*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].z); 
            if (zc < NUM_Z_OSCILLATORS-1) 
                        p2 = F3dVector(Oscillators[xc+(zc+1)*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+(zc+1)*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+(zc+1)*NUM_X_OSCILLATORS].z);
            else
                        p2 = F3dVector(Oscillators[xc+zc*NUM_X_OSCILLATORS].x,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].y,
                                       Oscillators[xc+zc*NUM_X_OSCILLATORS].z); 
            v = p2-p1; //vector from the upper neighbor to the lower neighbor
            //calculat the normal:
            SF3dVector normal = Normalize3dVector(CrossProduct(&u,&v));

            //assign the normal:
            Oscillators[xc+zc*NUM_X_OSCILLATORS].nx = normal.x;
            Oscillators[xc+zc*NUM_X_OSCILLATORS].ny = normal.y;
            Oscillators[xc+zc*NUM_X_OSCILLATORS].nz = normal.z;
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


//If key is pressed, rotate the teapots
void updateAngleAndDisplacement( int value )
{
    if(keydown) {
        glTranslatef(0, 0, camPosZ);
        glRotatef(1.0 * direction, 0.0, 1.0, 0.0);
        glTranslatef(0, 0, -camPosZ);
    }
    glutPostRedisplay();
    glutTimerFunc(20, updateAngleAndDisplacement, 0);
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
    g_timePassedSinceStart += dtime;

    if (g_timePassedSinceStart > 1.7f)
    {
        g_bExcitersInUse = false;  //stop the exciters
    }
//ENABLE THE FOLLOWING LINES FOR A RAIN EFFECT
    int randomNumber = rand();
    if (randomNumber < NUM_OSCILLATORS)
    {
        Oscillators[randomNumber].y = -0.05;
    }
    


    UpdateScene(false,dtime,g_timePassedSinceStart);
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

    CreatePool();
    //Enable the vertex array functionality:
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(    3,   //3 components per vertex (x,y,z)
                        GL_FLOAT,
                        sizeof(SOscillator),
                        Oscillators);
    glNormalPointer(    GL_FLOAT,
                        sizeof(SOscillator),
                        &Oscillators[0].nx);  //Pointer to the first color*/
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
    glutTimerFunc( 20, updateAngleAndDisplacement, 0 );
    glutIdleFunc(Idle);

    glutMainLoop();
    delete shaderProg;
}
