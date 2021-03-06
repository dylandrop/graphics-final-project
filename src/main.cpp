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
#include "SOIL.h"

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
bool up_or_down = false;
float up_or_down_direction = 1.0;
float direction = 1.0;

//simulation stuff
bool move_ball_downwards = false;
GLfloat ball_offset = 0.0;
GLfloat ball_velocity = 0.0;
GLfloat ball_acceleration = 0.0;
//ball will stay 1/3 way submerged in still water
GLfloat FLOATATION_COEFFICIENT = 1.5;
GLfloat GRAVITY_COEFFICIENT = 1.0; 
GLfloat BALL_RADIUS = 1.0;
GLfloat AIR_RESISTANCE_COEFFICIENT = 0.01;
GLfloat WATER_RESISTANCE_COEFFICIENT = 0.02;
bool out_of_water = false;
bool center_exciter_on = false;
GLfloat time_of_reenter_ripple = 0.0;

// Lights & Materials
GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat position[] = {0.3, 0.3, 0.3, 1.0};
GLfloat position2[] = {0.0, 0.3, 0.0, 1.0};
GLfloat mat_diffuse[] = {0.6, 0.6, 0.6, 1.0};
GLfloat mat_specular[] = {0.7, 0.7, 0.7, 1.0};
GLfloat waterColor[] = {0.0, 0.5, 0.8, 0.6};
GLfloat skyColor[] = {0.2, 0.3, 0.8, 1.0};

GLfloat mat_shininess[] = {50.0};

static GLScreenCapturer screenshot("screenshot-%d.ppm");
static GLSLProgram*     shaderProg = NULL;
static GLSLProgram* toon = NULL;
static GLSLProgram* gour = NULL;
static GLSLProgram* blinnp = NULL;
static GLSLProgram* checkbp = NULL;
static GLSLProgram* tex_2d_shader = NULL;
static GLSLProgram* bonus = NULL;
static GLuint tex_2d;
static GLUquadric *sphere;

#define TOTAL_I_VERTS       140
#define TOTAL_J_VERTS       140
#define TOTAL_VERTS         TOTAL_I_VERTS*TOTAL_J_VERTS
#define VERT_DISTANCE     0.05
#define BODY_SCALE_FACTOR 7.0
#define BODY_SIZE         TOTAL_I_VERTS*VERT_DISTANCE*BODY_SCALE_FACTOR
#define dT 0.003f

#define NORMALIZATION_FACTOR       0.0002
GLuint * vIdxs;
float initialtime = 0.0f;
int indexCount;
bool jump = true;

Vertex3 * verts;

void initWater()
{
    //the array of all the vertices themselves
    verts = new Vertex3[TOTAL_VERTS];
    //indices of all the vertices
    vIdxs = new GLuint[(TOTAL_VERTS - (TOTAL_I_VERTS + TOTAL_J_VERTS - 1))*6]; 
    int count = 0;
    for (int i = 0; i < TOTAL_I_VERTS; i++) {
        for (int j = 0; j < TOTAL_J_VERTS; j++) 
        {
            int thisVert = i+j*TOTAL_I_VERTS;

            verts[thisVert] = Vertex3::Make(VERT_DISTANCE*float(i)*BODY_SCALE_FACTOR, 0.0, 
                VERT_DISTANCE*float(j)*BODY_SCALE_FACTOR, 0, 1, 0, 
                0, false);

            if ((i < TOTAL_I_VERTS-1) && (j < TOTAL_J_VERTS-1))
            {
                //Make a triangle with the neighboring points
                vIdxs[count++] = thisVert;
                vIdxs[count++] = thisVert + 1;
                vIdxs[count++] = thisVert + TOTAL_I_VERTS + 1;

                vIdxs[count++] = thisVert;
                vIdxs[count++] = thisVert + TOTAL_I_VERTS + 1;
                vIdxs[count++] = thisVert + TOTAL_I_VERTS;
            } 

        }
    }
    
    // verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2+3].jump = true;
    // verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2+3].jumpAmt = 0.8;
    // verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2+3].jumpFreq = 100;

    //Excite the far edge and the right edge in the form of a sine wave.
    //The sin(i/PIPIPI) makes the exciters a smooth sine wave, sort of
    //like an ocean wave.

    for(int i = 0; i < TOTAL_I_VERTS - 2; i++) {
      //far edge
      verts[i+135*TOTAL_I_VERTS].jump = true;
      verts[i+135*TOTAL_I_VERTS].jumpAmt = 0.3f*sin(i/(PI*PI*PI));
      verts[i+135*TOTAL_I_VERTS].jumpFreq = 10.0f*sin(i/(PI*PI*PI)) + 10.0f;
      //right edge
      verts[TOTAL_I_VERTS-2+TOTAL_J_VERTS*i].jump = true;
      verts[TOTAL_I_VERTS-2+TOTAL_J_VERTS*i].jumpAmt = 0.41f*sin(i/(PI*PI*PI));
      verts[TOTAL_I_VERTS-2+TOTAL_J_VERTS*i].jumpFreq = 8.7f*sin(i/(PI*PI*PI)) + 11.1f;
    }
    indexCount = count;
}

//incrementally update the water
void refreshWater(float time)
{
  for (int i = 0; i < TOTAL_I_VERTS; i++) 
  {
    for (int j = 0; j < TOTAL_J_VERTS; j++) 
    {
      int thisVert = i+j*TOTAL_I_VERTS;

      //Handles the position of points that should be exciting the 
      //water around them.
      if ((verts[thisVert].jump) && jump) {
        verts[thisVert].newY = verts[thisVert].jumpAmt * sin(time*verts[thisVert].jumpFreq);
        //Cut them off if it's been enough time since the start of the program.
        if(time > 2.0f) {
          verts[thisVert].jumpAmt *= 0.5;
        }
      }

      if ((j==TOTAL_J_VERTS-1) || (j==0) || (i==TOTAL_I_VERTS-1) || (i==0))
        continue;
      else
      {
        //Average over your neighbor's distance, subtract 4 times your current distance/
        //Discussed in http://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf
        //(Heightfield approximations)
        GLfloat horizOffset = verts[thisVert-1].y + verts[thisVert+1].y;
        GLfloat vertOffset = verts[thisVert-TOTAL_I_VERTS].y + verts[thisVert+TOTAL_I_VERTS].y;
        GLfloat currentOffset = 4 * verts[thisVert].y;
        GLfloat deltaHeight = horizOffset + vertOffset - currentOffset;

        verts[thisVert].vel += deltaHeight * dT / NORMALIZATION_FACTOR;
        verts[thisVert].newY += verts[thisVert].vel * dT;
      }
    }       
  }

  for (int i = 0; i < TOTAL_I_VERTS; i++) 
  {
    for (int j = 0; j < TOTAL_J_VERTS; j++) 
    {
      int thisVert = i+j*TOTAL_I_VERTS;
      //Assign the height here as to not screw up the calculations of the
      //neighbors' new Y position.
      verts[thisVert].y = verts[thisVert].newY;

      Vector3 u,v;

      int leftVert = thisVert - 1;
      int rightVert = thisVert + 1;
      int upVert = thisVert - TOTAL_I_VERTS;
      int downVert = thisVert + TOTAL_I_VERTS;

      //The horizontal vector
      u = Vector3::Make(verts[leftVert].x, verts[leftVert].y, verts[leftVert].z) -
        Vector3::Make(verts[rightVert].x, verts[rightVert].y, verts[rightVert].z);

      //The vertical vector
      v = Vector3::Make(verts[upVert].x, verts[upVert].y, verts[upVert].z) - 
        Vector3::Make(verts[downVert].x, verts[downVert].y, verts[downVert].z);

      //The normal is approximately the cross product of these two vectors
      Vector3 norm = Vector3::Norm(Vector3::Cross(&u,&v));

      verts[thisVert].nX = norm.x; verts[thisVert].nY = norm.y; verts[thisVert].nZ = norm.z;
    }
  }

}

// Taken from http://www.lonesock.net/soil.html
int textLoad()
{
    tex_2d = SOIL_load_OGL_texture
        (
        "storm.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );
 
    if( 0 == tex_2d ) {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        return false;
    } 

    glBindTexture(GL_TEXTURE_2D, tex_2d);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void initLights(void)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, position2);
    
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
    glDrawElements( GL_TRIANGLES, 
      indexCount,
      GL_UNSIGNED_INT,
      vIdxs);;
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
    glMaterialfv(GL_FRONT, GL_DIFFUSE, waterColor);
    drawFloor();
    shaderProg->enable();

    shaderProg->bind_texture("texture", tex_2d, GL_TEXTURE_2D, 0);

    //pass the current time and light direction to the shaders
    shaderProg->set_uniform_1f("time", glutGet(GLUT_ELAPSED_TIME));
    shaderProg->set_uniform_3f("lightDir", 1.f, 1.f, 0.5f);
    // Draw two teapots next to each other in z axis

    
    glPushMatrix();
    {
        glTranslatef(0,-1.1,0);
        glTranslatef(0.0, ball_offset, 0.0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, unselectedColor);
        glLoadName(0);
        glutSolidSphere(BALL_RADIUS, 40, 40);
    }
    glPopMatrix();
    shaderProg->disable();
    GLSLProgram* temp = shaderProg;
    shaderProg = tex_2d_shader;
    shaderProg->enable();
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_DIFFUSE, skyColor);
    glRotatef(90,1,0,0);
    gluQuadricTexture(sphere, true);
    gluQuadricNormals(sphere, true);
    gluSphere(sphere, 50.5, 40, 40);
    shaderProg = temp;
    shaderProg->disable();
    glPopMatrix();
    
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
    case 'l':
        up_or_down = true; //rotate left
        up_or_down_direction = -1.0f;
        break;
    case 'o':
        up_or_down = true; //rotate right
        up_or_down_direction = 1.0f;
        break;
    case 'b':
        move_ball_downwards = true;
        break;
    }
}

//Key is released, stop rotating
void keyup(unsigned char key, int x, int y)
{
    keydown = false;
    up_or_down = false;
    move_ball_downwards = false;
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
    tex_2d_shader = new GLSLProgram(textureVS, textureFS);
    bonus = new GLSLProgram(bonusVS, bonusFS);
    shaderProg = blinnp;
}
void moveBall(){
    GLfloat force_coeff = 0.0;
    GLfloat ammount_submerged = BALL_RADIUS - (ball_offset -  verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].y);
    if (ammount_submerged>2)
        ammount_submerged = 2;
    if (ammount_submerged<0)
    {
        force_coeff = -GRAVITY_COEFFICIENT;
        out_of_water = true;
    }
    else{
        if(out_of_water && !center_exciter_on){
            center_exciter_on = true;
            verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].jump = true;
            if(initialtime>2.0){
                verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].jumpAmt = 0.8;
                verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].jumpFreq = 100.;
            }
            else{
                verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].jumpAmt = 0.4;
                verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].jumpFreq = 100.;    
            }
            
        }
        force_coeff = ammount_submerged*FLOATATION_COEFFICIENT - GRAVITY_COEFFICIENT;    
    }
    if (ammount_submerged>0.6){
        out_of_water = false;
        center_exciter_on = false;
        verts[TOTAL_I_VERTS/2+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].jump = false;

    }
    //friction
    if(ammount_submerged<0){
        //case in the air
        force_coeff -= ball_velocity*AIR_RESISTANCE_COEFFICIENT;
    }
    else{
        //case in water
        force_coeff-= ball_velocity*WATER_RESISTANCE_COEFFICIENT;
        
    }
    
    ball_velocity += force_coeff*dT*500;
    ball_offset += ball_velocity*dT;
    // printf("%f force coefficient\n", force_coeff);
    // printf("%f ball_velocity\n", ball_velocity);
    //ball_offset = -verts[TOTAL_I_VERTS/4+ TOTAL_J_VERTS*TOTAL_I_VERTS/2].y;

    
}

void idle(void)
{
  initialtime += dT;

  if(up_or_down){
      glTranslatef(0, 0, camPosZ);
      glRotatef(1.0 * up_or_down_direction, 1.0, 0.0, 0.0);
      glTranslatef(0, 0, -camPosZ);   
  }

  if(keydown) {
    glTranslatef(0, 0, camPosZ);
    glRotatef(1.0 * direction, 0.0, 1.0, 0.0);
    glTranslatef(0, 0, -camPosZ);
  }
  
  refreshWater(initialtime);
  if(move_ball_downwards){
    ball_offset -= 0.1;
  }
  else
    moveBall();
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
    textLoad();
    setupShaders();
    setupRC();

    initWater();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer( 3,
      GL_FLOAT,
      sizeof(Vertex3),
      verts);
    glNormalPointer( GL_FLOAT,
      sizeof(Vertex3),
      &verts[0].nX);
    glPointSize(2.0);
    glClearColor(0.0,1.0,1.0,0.0);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutKeyboardUpFunc( keyup );
    sphere = gluNewQuadric();
    glutMouseFunc( mouse );
    glutMotionFunc( motion );
    glutIdleFunc(idle);

    glutMainLoop();
    delete shaderProg;
}
