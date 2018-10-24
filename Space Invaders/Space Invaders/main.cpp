/*
 * trySI.cpp
 *
 *  Created on: Sep 19, 2018
 *      Author: paul.mullins
 *
 *  Bare bones game structure for "space invaders"
 *  First... understand the code. Make changes to see that you have it.
 *
 *  Modifications needed
 *  checking for a hit and then dealing with it
 *     make the alien hit "blow up" - new animation
 *     the cannon should be able to fire again as soon as a hit occurs
 *  Checking for all aliens defeated and then dealing with it
 *  Ending animation, win or lose, whatever that may be
 *
 *  You may make other changes as you see fit to create an interactive game
 *  However, don't create a "2D screen"... there is a good chance we will
 *     modify the game to be 3D later (ROWS, COLS, RANKs).
 *     This means that if you are displaying text (or status) at all, you probably
 *     want it to be able to move with the camera (thus "appearing" stationary in
 *     the window).
 */

///////////////////////////////////////////////////////////////////////////////////
// Code pirated from spaceTravel.cpp
//
// Interaction:
// Press the left/right arrow keys to move the cannon.
//
// Sumanta Guha.
///////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
//#  include <GL/glew.h>
#  include <GLUT/GLUT.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/glext.h>
#pragma comment(lib, "glew32.lib")
#endif

#define PI 3.14159265
static unsigned int animationPeriod = 1000; // Time interval between frames.
static float bottom = -80.0; // lowest y-coord for aliens (see animate)
static float top = 140; // top of "screen" (for cannon ball)
static float edge = 140.0; // a virtual "edge of screen" (x-coord checked in animate)
static float deltaX = 5.0; // speed for an alien
static float zPlane = -60.0; // where all the action is
static float yCannon = -100.0; // translate for cannon
static int gameOver = 0; // if game is over, probably started a final animation
static int canShoot = 1; // cannon ready to fire
int score = 0; //score of the game
int hit, alienHiti, alienHitj, alienHitk = 0;
float blow = 3;


#define ROWS 4  // Number of rows of Aliens.
#define COLUMNS 8 // Number of columns of Aliens.
#define AISLES 4 // Number of layers (z) of aliens

using namespace std;

// Globals.
static float xVal = 0; // Co-ordinates of the cannon (xVal, 0, zPlane)
//static int isCollision = 0; // Is there collision between the cannonball and an Alien?
static unsigned int cannon; // Display lists base index.

// Alien class.
class Alien
{
public:
    Alien();
    Alien(float x, float y, float z, float r, unsigned char colorR,
          unsigned char colorG, unsigned char colorB);
    float getCenterX() { return centerX; }
    float getCenterY() { return centerY; }
    float getCenterZ() { return centerZ; }
    float getRadius()  { return radius; }
    void draw();
    void adjustCenter(float x, float y, float z);
    void setCenter(float x, float y, float z);
    void setRadius(float r) { radius = r; }
    
private:
    float centerX, centerY, centerZ, radius;
    unsigned char color[3];
};

// Alien default constructor.
Alien::Alien()
{
    centerX = 0.0;
    centerY = 0.0;
    centerZ = 0.0;
    radius = 0.0; // Indicates no Alien exists in the position.
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
}

// Alien constructor.
Alien::Alien(float x, float y, float z, float r, unsigned char colorR,
             unsigned char colorG, unsigned char colorB)
{
    centerX = x;
    centerY = y;
    centerZ = z;
    radius = r;
    color[0] = colorR;
    color[1] = colorG;
    color[2] = colorB;
}
void alienHit(int i, int j, int k, float x);

// Alien change location
void Alien::adjustCenter(float x, float y, float z)
{
    centerX += x;
    centerY += y;
    centerZ += z;
}

// Alien set location
void Alien::setCenter(float x, float y, float z)
{
    centerX = x;
    centerY = y;
    centerZ = z;
}

// Function to draw Alien.
void Alien::draw()
{
    if (radius > 0.0) // If Alien exists.
    {
        glPushMatrix();
        glTranslatef(centerX, centerY, centerZ);
        glColor3ubv(color);
        glutWireSphere(radius, (int)radius*6, (int)radius*6);
        glPopMatrix();
    }
}

Alien arrayAliens[ROWS][COLUMNS][AISLES]; // Global array of Aliens.
Alien cannonBall;


// Function to check if two spheres centered at (x1,y1,z1) and (x2,y2,z2) with
// radius r1 and r2 intersect.
int checkSpheresIntersection(float x1, float y1, float z1, float r1,
                             float x2, float y2, float z2, float r2)
{
    return ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) <= (r1+r2)*(r1+r2) );
}

// Function to check if the cannon collides with an Alien when the center of the base
// of the craft is at (x, 0, z) and it is aligned at an angle a to to the -z direction.
// Collision detection is approximate as instead of the cannon we use a bounding sphere.
int AlienCraftCollision(float x, float z, float a)
{
    int i,j,k;
    
    // Check for collision with each Alien.
    for (j=0; j<COLUMNS; j++)
        for (i=0; i<ROWS; i++)
            for (k=0; k<AISLES; k++)
                if (arrayAliens[i][j][k].getRadius() > 0 ){ // If Alien exists.
                    if ( checkSpheresIntersection( x - 5 * sin( (PI/180.0) * a), 0.0,
                                              z - 5 * cos( (PI/180.0) * a), 7.072,
                                              arrayAliens[i][j][k].getCenterX(), arrayAliens[i][j][k].getCenterY(),
                                              arrayAliens[i][j][k].getCenterZ(), arrayAliens[i][j][k].getRadius() ) ){
                    return 1;
                    
                }
            }
    return 0;
}

void displayEndText(string message){
    // Get the length to declare the array size
    int n = message.length();
    // The character array to hole the string
    char text[n+1];
    // Copying the string to the array of characters
    strcpy(text, message.c_str());
    // The color, red for me
    glColor3f(255, 255, 255);
    // Position of the text to be printer
    glRasterPos3f(-20, 110, 0);
    for(int i = 0; text[i] != '\0'; i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void displayScore(){
    string strScore = "Score: " + to_string(score);
    
    int n = strScore.length();
    char text[n+1];
    strcpy(text, strScore.c_str());
    
    glColor3f(255, 255, 255);
    glRasterPos3f(-38, -30, 0);
    
    for(int i = 0; text[i] != '\0'; i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// Drawing routine.
void drawScene(void)
{
    
    int i, j, k;
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    
//    gluLookAt(0.0, 5.0, 20.0,  0.0, 5.0, 0.0,  0.0, 1.0, 0.0);
        gluLookAt(0.0, -150.0, 5.0,  0.0, 20.0, 0.0,  0.0, 1.0, 0.0);
    //    gluLookAt(125.0, -90.0, zPlane,  0.0, -90.0, zPlane,  0.0, 1.0, 0.0); // side view
    //    gluLookAt(25.0, -100.0, zPlane,  0.0, -100.0, zPlane,  0.0, 1.0, 0.0); // side close up
    
    displayScore(); //displays the score on the screen
    
    // Draw all the Aliens (with radius > 0) in arrayAliens
    for (j=0; j<COLUMNS; j++)
        for (i=0; i<ROWS; i++)
            for (k=0; k<AISLES; k++)
                arrayAliens[i][j][k].draw();
    
    cannonBall.draw(); // only appears if r>0
    
    glTranslatef(xVal, yCannon, zPlane);
    glCallList(cannon);
    
    if(score >= 320){
        gameOver = 1;
        displayEndText("Winner!");
    }
    else if(gameOver && score < 320){
        displayEndText("You Lose!");
    }
    
    if(hit == 1){
        alienHit(alienHiti, alienHitj, alienHitk, blow);
        //        cout << "\nblow in draw scene = ";
        //        cout << blow, "\n";
    }
    
    
    glFlush();
    glutSwapBuffers();
    
}

void animateEndGame(int value)
{
    cout << "You lose" << endl;
    // if this is really an animation, do your thing and set a timer
}

// Routine to animate with a recursive call made after animationPeriod msecs.
void animateAliens(int value)
{
    float deltaY = 0.0;
    if (!gameOver)
    {
        // move aliens
        if ((arrayAliens[0][0][0].getCenterX() <= -edge && deltaX < 0) ||
            (arrayAliens[0][COLUMNS-1][0].getCenterX() >= edge && deltaX > 0))
        { // step down and reverse direction
            deltaY = -5 * abs(deltaX);  // adjust step down here
            deltaX = -deltaX;
        }
        
        for (int j=0; j<COLUMNS; j++)
            for (int i=0; i<ROWS; i++)
                for (int k=0; k<AISLES; k++)
                {
                    // if this alien is "marked" then deal... at least set radius to 0
                    if (arrayAliens[i][j][k].getRadius() > 0 &&
                        arrayAliens[i][j][k].getCenterY() + deltaY  <= bottom)
                        gameOver = 1; // lose on next cycle
                    arrayAliens[i][j][k].adjustCenter(deltaX, deltaY, 0.0);
                }
    }
    
    glutPostRedisplay();
    
    if (!gameOver) // currently only one way for game to be "over"
        glutTimerFunc(animationPeriod, animateAliens, 1);
    else
        glutTimerFunc(animationPeriod, animateEndGame, 1);
    
}

// Initialization routine.
void setup(void)
{
    int i, j, k;
    
    cannon = glGenLists(1);
    glNewList(cannon, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.0, -1.0, 0.0, 0.0); // To make the cannon point up the y-axis
    glColor3f (1.0, 1.0, 1.0);
    glutWireCone(5.0, 10.0, 10, 10);
    glPopMatrix();
    glEndList();
    
    cannonBall = Alien(0.0, 0.0, 0.0, 0, 255, 0, 255);
    
    // Initialize global arrayAliens.
    for (j=0; j<COLUMNS; j++)
        for (i=0; i<ROWS; i++)
            for (k=0; k<AISLES; k++)
                // assume even number of columns.
                // x,y,z r (2..3 radius)  RGB
                arrayAliens[i][j][k] = Alien( 15 + 30.0*(-COLUMNS/2 + j), 120.0 - 30.0*i, zPlane,
                                             (double) (rand() % 2 + 2),
                                             188, 188, 0);
    
    glEnable(GL_DEPTH_TEST);
    glClearColor (0.0, 0.0, 0.0, 0.0);
    
    animateAliens(1); // start the animation
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport (0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-10.0, 10.0, -10.0, 10.0, 5.0, 250.0);
    glMatrixMode(GL_MODELVIEW);
}

void alienHit(int i, int j, int k, float x){
    
    float alienHitRadius = arrayAliens[i][j][k].getRadius();
    //    cout << alienHitRadius; //debug
    
    //    alienHitRadius *= blow;
    //    arrayAliens[i][j][k].setRadius(alienHitRadius);
    
    if(blow == 3){
        alienHitRadius *= blow;
        arrayAliens[i][j][k] = Alien(arrayAliens[i][j][k].getCenterX(), arrayAliens[i][j][k].getCenterY(), arrayAliens[i][j][k].getCenterZ(), arrayAliens[i][j][k].getRadius(), 255, 0, 0);
        arrayAliens[i][j][k].setRadius(alienHitRadius);
        //        cout << "\ndebug if 1-> ";
        //        cout << blow, "\n";
        blow = 1;
        //        cout << "\ndebug if 2-> ";
        //        cout << blow, "\n";
    }
    else if(blow == 1){
        arrayAliens[i][j][k].setRadius(0);
        //        cout << "\ndebug else if 1-> ";
        //        cout <<  blow, "\n";
        blow = 3;
        //        cout << "\ndebug else if 2-> ";
        //        cout << blow, "\n";
        hit = 0;
    }
    //    cout << alienHitRadius; //debug
    //    arrayAliens[i][j][k].setRadius(0);
}

void animateCannonBall(int value)
{
    float canX;
    float canY;
    float canZ;
    float canRad;
    int i,j,k;
    
    if (cannonBall.getCenterY() < top)
    {
        cannonBall.adjustCenter(0,1,0);
        // check for a hit
        canX = cannonBall.getCenterX();
        canY = cannonBall.getCenterY();
        canZ = cannonBall.getCenterZ();
        canRad = cannonBall.getRadius();
        // Check for collision with each Alien
        for (j=0; j<COLUMNS; j++)
            for (i=0; i<ROWS; i++)
                for (k=0; k<AISLES; k++)
                    if (arrayAliens[i][j][k].getRadius() > 0 ){ // If Alien exists.
                        if ( checkSpheresIntersection(canX, canY, canZ, canRad,
                                                      arrayAliens[i][j][k].getCenterX(), arrayAliens[i][j][k].getCenterY(),
                                                      arrayAliens[i][j][k].getCenterZ(), arrayAliens[i][j][k].getRadius())){
                            cannonBall.setRadius(0);
                            score += 10;
                            cannonBall.adjustCenter(0, 141, 0);
                            canShoot = 1;
                            
                            alienHiti = i;
                            alienHitj = j;
                            alienHitk = k;
                            
                            hit = 1;
                        }
                    }
        
        glutPostRedisplay();
        glutTimerFunc(5, animateCannonBall, 1);
    }
    else
    {
        cannonBall.setRadius(0);
        canShoot = 1;
    }
    
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
            // use space key to shoot upwards from cannon location
        case ' ':
            if (canShoot)
            {
                canShoot = 0; // need a delay here
                cannonBall.setRadius(1); // displayable
                cannonBall.setCenter(xVal, yCannon+5, zPlane);
                animateCannonBall(1);
                //glutTimerFunc(50, animateCannonBall, 1);
            }
            break;
        case 27:
            exit(0);
            break;
        default:
            break;
    }
}

// Callback routine for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
    float tempxVal = xVal;
    
    // Compute next position.
    if (key == GLUT_KEY_LEFT) tempxVal = xVal - 5.0;
    if (key == GLUT_KEY_RIGHT) tempxVal = xVal + 5.0;
    
    // Move cannon to next position only if not offscreen.
    if (abs(tempxVal) <= edge)
    {
        xVal = tempxVal;
    }
    
    glutPostRedisplay();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
    cout << "Interaction:" << endl;
    cout << "Press the left/right arrow keys to turn the craft." << endl
    << "Press the up/down arrow keys to move the craft." << endl;
}

// Main routine.
int main(int argc, char **argv)
{
    printInteraction();
    glutInit(&argc, argv);
    
    //    glutInitContextVersion(4, 0);
    //    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("spaceTravel.cpp");
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutSpecialFunc(specialKeyInput);
    
    //    glewExperimental = GL_TRUE;
    //    glewInit();
    
    setup();
    
    glutMainLoop();
}
