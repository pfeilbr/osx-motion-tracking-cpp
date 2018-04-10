//
//  main.cpp
//  osx-motion-tracking-cpp
//
//  Created by Brian Pfeil on 4/7/18.
//  Copyright © 2018 Brian Pfeil. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <GLUT/glut.h>

using namespace cv;
using namespace std;

/* global variables */
CvCapture* capture = 0;
IplImage* imgScribble = NULL;
double rotate_y=0;
double rotate_x=0;
static int posX = 0;
static int posY = 0;

void showImage() {
    Mat image;
    const char *imagePath = "/Users/brianpfeil/Downloads/opencv-logo-white.png";
    image = imread(imagePath, CV_LOAD_IMAGE_COLOR);   // Read the file
    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "Display window", image );                   // Show our image inside it.
    waitKey(0);
}

void showVideo() {
    VideoCapture camera(0); // open the default camera
    if(!camera.isOpened()) {
        cout << "could not open camera" << endl;
        return;
    }
    
    Mat processedImg;
    namedWindow("processedImg",1);
    for(;;)
    {
        Mat cameraImg;
        camera.read(cameraImg);
        //camera >> cameraImg; // get a new frame from camera
        cvtColor(cameraImg, processedImg, COLOR_BGR2GRAY);
        //GaussianBlur(processedImg, processedImg, Size(7,7), 1.5, 1.5);
        //Canny(processedImg, processedImg, 0, 30, 3);
        imshow("processedImg", processedImg);
        if(waitKey(5) >= 0) break;
    }
}

IplImage* GetThresholdedImage(IplImage* img)
{
    // Convert the image into an HSV image
    IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
    cvCvtColor(img, imgHSV, CV_BGR2HSV);
    IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);
    cvInRangeS(imgHSV, cvScalar(20, 100, 100), cvScalar(30, 255, 255), imgThreshed);
    cvReleaseImage(&imgHSV);
    return imgThreshed;
}

int colorTracking() {
    // Initialize capturing live feed from the camera
    CvCapture* capture = 0;
    
    capture = cvCaptureFromCAM(0);
    
    // Couldn't get a device? Throw an error and quit
    if(!capture)
    {
        printf("Could not initialize capturing...");
        return -1;
    }
    
    // The two windows we'll be using
    cvNamedWindow("video");
    cvNamedWindow("thresh");
    IplImage* imgScribble = NULL;
    // An infinite loop
    while(true)
    {
        // Will hold a frame captured from the camera
        IplImage* frame = 0;
        frame = cvQueryFrame(capture);
        if(!frame)
            break;
        // If this is the first frame, we need to initialize it
        if(imgScribble == NULL)
        {
            imgScribble = cvCreateImage(cvGetSize(frame), 8, 3);
        }
        IplImage* imgYellowThresh = GetThresholdedImage(frame);
        // Calculate the moments to estimate the position of the ball
        CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
        
        cvMoments(imgYellowThresh, moments, 1);
        
        // The actual moment values
        double moment10 = cvGetSpatialMoment(moments, 1, 0);
        double moment01 = cvGetSpatialMoment(moments, 0, 1);
        
        double area = cvGetCentralMoment(moments, 0, 0);
        // Holding the last and current ball positions
        static int posX = 0;
        
        static int posY = 0;
        
        int lastX = posX;
        
        int lastY = posY;
        
        posX = moment10/area;
        posY = moment01/area;
        // Print it out for debugging purposes
        printf("position (%d,%d)", posX, posY);
        // We want to draw a line only if its a valid position
        if(lastX>0 && lastY>0 && posX>0 && posY>0)
        {
            // Draw a yellow line from the previous point to the current point
            cvLine(imgScribble, cvPoint(posX, posY), cvPoint(lastX, lastY), cvScalar(0,255,255), 5);
            
        }
        // Add the scribbling image and the frame...
        cvAdd(frame, imgScribble, frame);
        cvShowImage("thresh", imgYellowThresh);
        cvShowImage("video", frame);
        // Wait for a keypress
        int c = cvWaitKey(10);
        if(c!=-1)
        {
            // If pressed, break out of the loop
            break;
        }
        // Release the thresholded image+moments... we need no memory leaks.. please
        cvReleaseImage(&imgYellowThresh);
        delete moments;
    }    // We're done using the camera. Other applications can now use it
    cvReleaseCapture(&capture);
    return 0;
}

void on_opengl(void* param)
{
    glLoadIdentity();
    glTranslated(0.0, 0.0, -1.0);
    glRotatef( 55, 1, 0, 0 );
    glRotatef( 45, 0, 1, 0 );
    glRotatef( 0, 0, 0, 1 );
    static const int coords[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };
    for (int i = 0; i < 6; ++i) {
        glColor3ub( i*20, 100+i*10, i*42 );
        glBegin(GL_QUADS);
        for (int j = 0; j < 4; ++j) {
            glVertex3d(0.2 * coords[i][j][0], 0.2 * coords[i][j][1], 0.2 * coords[i][j][2]);
        }
        glEnd();
    }
}

void openCVopenGLExample()
{
    cv::namedWindow( "OpenGlRender", CV_WINDOW_OPENGL );
    cv::setOpenGlDrawCallback("OpenGlRender", on_opengl, NULL);
    cv::updateWindow("OpenGlRender");
    cv::waitKey(0);
}

void renderFrameCallback() {
    
    // opencv stuff here
    
    // Will hold a frame captured from the camera
    IplImage* frame = 0;
    frame = cvQueryFrame(capture);

    if(imgScribble == NULL) {
        imgScribble = cvCreateImage(cvGetSize(frame), 8, 3);
    }
    IplImage* imgYellowThresh = GetThresholdedImage(frame);

    CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
    cvMoments(imgYellowThresh, moments, 1);
    double moment10 = cvGetSpatialMoment(moments, 1, 0);
    double moment01 = cvGetSpatialMoment(moments, 0, 1);
    double area = cvGetCentralMoment(moments, 0, 0);

    int lastX = posX;
    int lastY = posY;

    posX = moment10/area;
    posY = moment01/area;
    
    if(lastX>0 && lastY>0 && posX>0 && posY>0) {
        cvLine(imgScribble, cvPoint(posX, posY), cvPoint(lastX, lastY), cvScalar(0,255,255), 5);
    }
    
    cvAdd(frame, imgScribble, frame);
    cvShowImage("thresh", imgYellowThresh);
    cvShowImage("video", frame);
    
    /*
    int c = cvWaitKey(1);
    if(c != -1) {
        printf("key pressed. exiting\n");
        return;
    }
    */

    cvReleaseImage(&imgYellowThresh);
    delete moments;

    glutPostRedisplay();
    
    //  Clear screen and Z-buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    // Reset transformations
    glLoadIdentity();
    
    glTranslatef(((GLfloat)posX/-1280.0), ((GLfloat)posY/-720.0), -1.0);
    
    // Other Transformations
    // glTranslatef( 0.1, 0.0, 0.0 );      // Not included
    // glRotatef( 180, 0.0, 1.0, 0.0 );    // Not included
    
    // Rotate when user changes rotate_x and rotate_y
    glRotatef( rotate_x, 1.0, 0.0, 0.0 );
    glRotatef( rotate_y, 0.0, 1.0, 0.0 );
    
    // Other Transformations
    // glScalef( 2.0, 2.0, 0.0 );          // Not included
    
    //Multi-colored side - FRONT
    glBegin(GL_POLYGON);
    
    glColor3f( 1.0, 0.0, 0.0 );     glVertex3f(  0.5, -0.5, -0.5 );      // P1 is red
    glColor3f( 0.0, 1.0, 0.0 );     glVertex3f(  0.5,  0.5, -0.5 );      // P2 is green
    glColor3f( 0.0, 0.0, 1.0 );     glVertex3f( -0.5,  0.5, -0.5 );      // P3 is blue
    glColor3f( 1.0, 0.0, 1.0 );     glVertex3f( -0.5, -0.5, -0.5 );      // P4 is purple
    
    glEnd();
    
    // White side - BACK
    glBegin(GL_POLYGON);
    glColor3f(   1.0,  1.0, 1.0 );
    glVertex3f(  0.5, -0.5, 0.5 );
    glVertex3f(  0.5,  0.5, 0.5 );
    glVertex3f( -0.5,  0.5, 0.5 );
    glVertex3f( -0.5, -0.5, 0.5 );
    glEnd();
    
    // Purple side - RIGHT
    glBegin(GL_POLYGON);
    glColor3f(  1.0,  0.0,  1.0 );
    glVertex3f( 0.5, -0.5, -0.5 );
    glVertex3f( 0.5,  0.5, -0.5 );
    glVertex3f( 0.5,  0.5,  0.5 );
    glVertex3f( 0.5, -0.5,  0.5 );
    glEnd();
    
    // Green side - LEFT
    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0,  0.0 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();
    
    // Blue side - TOP
    glBegin(GL_POLYGON);
    glColor3f(   0.0,  0.0,  1.0 );
    glVertex3f(  0.5,  0.5,  0.5 );
    glVertex3f(  0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glEnd();
    
    // Red side - BOTTOM
    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0,  0.0 );
    glVertex3f(  0.5, -0.5, -0.5 );
    glVertex3f(  0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    
    glEnd();
    
    glFlush();
    glutSwapBuffers();
    

}

void specialKeys( int key, int x, int y ) {
    
    //  Right arrow - increase rotation by 5 degree
    if (key == GLUT_KEY_RIGHT)
        rotate_y += 5;
    
    //  Left arrow - decrease rotation by 5 degree
    else if (key == GLUT_KEY_LEFT)
        rotate_y -= 5;
    
    else if (key == GLUT_KEY_UP)
        rotate_x += 5;
    
    else if (key == GLUT_KEY_DOWN)
        rotate_x -= 5;
    
    //  Request display update
    glutPostRedisplay();
    
}

void openCVAndOpenGL()
{
    // *** opencv ***
    capture = cvCaptureFromCAM(0);
    if(!capture) {
        printf("Could not initialize capturing...");
        return;
    }

    // *** opengl **
    int num = 0;
    glutInit(&num, NULL);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello world :D");
    
    cvNamedWindow("video", WINDOW_OPENGL);
    cvNamedWindow("thresh", WINDOW_OPENGL);

    glutDisplayFunc(renderFrameCallback);
    glutSpecialFunc(specialKeys);
    glutMainLoop();
}


int main(int argc, const char * argv[]) {
    //showImage();
    //showVideo();
    //colorTracking();
    openCVAndOpenGL();
    //openCVopenGLExample();
    return 0;
}
