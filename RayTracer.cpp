/*==================================================================================
* COSC 363  Computer Graphics (2023)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab06.pdf   for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "cylinder.h"
#include "cone.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
TextureBMP texture;
bool fog = false;
bool antialiasing = true;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
    glm::vec3 lightPos(0, 29, -40);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

    if (ray.index == 3)
    {
        // Checkered pattern
        int checkSize = 5; // Size of each checkered square
        int ix = (ray.hit.x + 60) / checkSize;
        int iz = (ray.hit.z) / checkSize;
        int k = (ix + iz) % 2; // 2 colors alternate based on sum of indices

        if (k == 0)
            color = glm::vec3(1, 1, 1); // White
        else
            color = glm::vec3(0.5, 0.5, 0.5); // Gray

        obj->setColor(color);
    }

    // Textured Sphere
    if (ray.index == 0) {
        glm::vec3 normal = obj->normal(ray.hit);
        float textcoords = 0.5 + ((atan2(normal.z,normal.x)) / (2*M_PI));
        float textcoordt = 0.5 + ((asin(normal.y)) / (M_PI));
        color = texture.getColorAt(textcoords, textcoordt);
        obj->setColor(color);
    }

    color = obj->lighting(lightPos, -ray.dir, ray.hit);

    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);

    if ((shadowRay.index > -1) && (shadowRay.dist < glm::length(lightVec)) && (shadowRay.index != ray.index)) {

        SceneObject* hitObject = sceneObjects[shadowRay.index];
        if (hitObject->isRefractive() || hitObject->isTransparent()) {
            color = 0.85f * obj->getColor() + 0.1f*hitObject->getColor();
        } else {
            color = 0.2f * obj->getColor(); //0.2 = ambient scale factor
        }
    }

    if (obj->isReflective() && step < MAX_STEPS) {
        float rho = obj->getReflectionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
        color = color + (rho * reflectedColor);
    }

    if (obj->isTransparent() && step < MAX_STEPS) {
        float rho = obj->getTransparencyCoeff();
        Ray reflectedRay(ray.hit, ray.dir);
        reflectedRay.closestPt(sceneObjects);
        Ray secondaryRay(reflectedRay.hit, ray.dir);
        glm::vec3 reflectedColor = trace(secondaryRay, step + 1);
        color = ((1 - rho) * color) + (rho * reflectedColor);
    }

    if (obj->isRefractive() && step < MAX_STEPS) {

        glm::vec3 n = obj->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, n, 1/1.01f);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        glm::vec3 m = obj->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f/(1/1.01f));

        Ray secondaryRay(refrRay.hit, h);
        glm::vec3 refractedColor = trace(secondaryRay, step + 1);
        color = (color) + (refractedColor);
    }

    if (fog) {
        float lambda = (ray.hit.z + 50.0f) / -200.0f;
        lambda = glm::clamp(lambda, 0.0f, 1.0f);
        color = ((1- lambda) * color + (lambda * glm::vec3(1,1,1)));
    }
	return color;
}


glm::vec3 antiAliasing(float xp, float cellX, float yp, float cellY, glm::vec3 eye) {

    glm::vec3 colorSum(0);
    Ray rays[4];

    glm::vec3 direction1(xp +  0.25 * cellX, yp +  0.25 * cellY, -EDIST);
    rays[0] = Ray(eye, direction1);
    glm::vec3 direction2(xp +  0.25 * cellX, yp + 0.75 * cellY, -EDIST);
    rays[1] = Ray(eye, direction1);
    glm::vec3 direction3(xp + 0.75 * cellX, yp + 0.75 * cellY, -EDIST);
    rays[2] = Ray(eye, direction1);
    glm::vec3 direction4(xp + 0.75 * cellX, yp +  0.25 * cellY, -EDIST);
    rays[3] = Ray(eye, direction4);

    for (int i = 0; i < 4; i++) {
        colorSum += trace(rays[i], i);
    }

    colorSum *= 0.25;
    return colorSum;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
    glm::vec3 eye(0, 0., 40.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
    {
        xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j * cellY;

            glm::vec3 col;
            if (antialiasing) {
                col = antiAliasing(xp, cellX, yp, cellY, eye);
            } else {
                glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray
                Ray ray = Ray(eye, dir);
                col = trace(ray, 1); //Trace the primary ray and get the colour value
            }

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    texture = TextureBMP("Disco.bmp");

    // Textured Sphere
    Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -90.0), 4.0);
    sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
    sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

    // Middle Sphere
    Sphere *sphere2 = new Sphere(glm::vec3(5.0, 5.0, -70.0), 4.0);
    sphere2->setColor(glm::vec3(1, 0, 0));   //Set colour to blue
    sphere2->setShininess(5);
    sphere2->setTransparency(true, 0.8);
    sceneObjects.push_back(sphere2);		 //Add sphere to scene objects

    // Bottom Sphere
    Sphere *sphere3 = new Sphere(glm::vec3(0.0, -10.0, -45.0), 5.0);
    sphere3->setColor(glm::vec3(0, 0, 0));
    sphere3->setRefractivity(true);
    sceneObjects.push_back(sphere3);

    // Ground
    Plane *plane = new Plane(glm::vec3(-40., -15, 40),
                             glm::vec3(40., -15, 40),
                             glm::vec3(40., -15, -220),
                             glm::vec3(-40., -15, -220));
    sceneObjects.push_back(plane);

    // Roof
    Plane *plane1 = new Plane(glm::vec3(-40., 30, 40),
                              glm::vec3(40., 30, 40),
                              glm::vec3(40., 30, -220),
                              glm::vec3(-40., 30, -220));
    plane1->setColor(glm::vec3(1,1,1));
    plane1->setSpecularity(false);
    sceneObjects.push_back(plane1);

    // Left Wall
    Plane *plane2 = new Plane(glm::vec3(-40., 30, 40),
                              glm::vec3(-40., -15, 40),
                              glm::vec3(-40., -15, -220),
                              glm::vec3(-40., 30, -220));
    plane2->setColor(glm::vec3(0.8,0,0));
    plane2->setSpecularity(false);
    sceneObjects.push_back(plane2);

    // Right Wall
    Plane *plane3 = new Plane(glm::vec3(40., 30, 40),
                              glm::vec3(40., 30, -220),
                              glm::vec3(40., -15, -220),
                              glm::vec3(40., -15, 40));
    plane3->setColor(glm::vec3(0,0,0.8));
    plane3->setSpecularity(false);
    sceneObjects.push_back(plane3);

    // Back Wall
    Plane *plane4 = new Plane(glm::vec3(-40., 30, -220),
                              glm::vec3(-40., -15, -220),
                              glm::vec3(40., -15, -220),
                              glm::vec3(40., 30, -220));
    plane4->setColor(glm::vec3(0,0,0));
    plane4->setSpecularity(false);
    plane4->setReflectivity(true);
    sceneObjects.push_back(plane4);

    // Behind Wall
    Plane *plane5 = new Plane(glm::vec3(-40., 30, 40),
                              glm::vec3(40., 30, 40),
                              glm::vec3(40., -15, 40),
                              glm::vec3(-40., -15, 40));
    plane5->setColor(glm::vec3(0,0,0));
    plane5->setReflectivity(true);
    sceneObjects.push_back(plane5);

    // Cylinder
    Cylinder *cylinder = new Cylinder(glm::vec3(18.0, -15.0, -60.0), 5.0, 5.0);
    cylinder->setColor(glm::vec3(0,0,1));
    sceneObjects.push_back(cylinder);

    // Cone
    Cone *cone = new Cone(glm::vec3(-30.0, -15.0, -160.0), 5.0, 10.0);
    cone->setColor(glm::vec3(0,1,0));
    sceneObjects.push_back(cone);
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
