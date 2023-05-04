/*==================================================================================
* COSC 363  Computer Graphics (2022)
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
#include "Cone.h"
#include "Cylinder.h"
#include "Plane.h"
#include "SceneObject.h"
#include "Ray.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

#define MIN_FOG 50
#define MAX_FOG 300

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
const float floorHalfWidth = 60;
bool antiAliasing = false;
bool fog = false;
void box();

vector<SceneObject*> sceneObjects;

TextureBMP texture;
TextureBMP earthTexture;

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(10, 40, -3);					//Light's position
    glm::vec3 colour(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

    if (ray.index == 1) {
        glm::vec3 n = obj->normal(ray.hit);
        float u = 0.5 + atan2(n.x, n.z) / (2 * M_PI);
        float v = 0.5 + asin(n.y) / M_PI;
        colour = earthTexture.getColorAt(u, v);
        obj->setColour(colour);
    }

    if (ray.index == 9) {
         //Checker pattern
         int checkSize = 2;
         int iz = (ray.hit.z) / checkSize;
         int ix = (ray.hit.x) / checkSize + 50;
         int k = (iz +  ix) % 2; //2 colours
         colour = (k != 0) ? glm::vec3(1, 0, 0.92) : colour = glm::vec3(1, 1, 0.5);
         obj->setColour(colour);
    }

    if (ray.index == 4) {
         //2 Patterns
         int checkSize = 1;
         int iz = (ray.hit.z) / checkSize + 50;
         int ix = (ray.hit.x) / checkSize + 50;
         int k = (ix > 45) ? (iz * ix + ix * ix) % 5 : (iz + ix * ix) % 5; //2 colours
         colour = (k != 0) ? glm::vec3(1, 0, 0.92) : glm::vec3(1, 1, 0.5);
         obj->setColour(colour);
    }

    if (ray.index == 11) {
        float texcoords = (ray.hit.x + floorHalfWidth)/(2 * floorHalfWidth);
        float texcoordt = (ray.hit.y + 15)/(75);
        if(texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1) obj->setColour(texture.getColorAt(texcoords, texcoordt));
    }

    if (ray.index == 16) {
        glm::vec3 n = obj->normal(ray.hit);
        float u = 0.5 + atan2(n.x, n.z) / (2 * M_PI);
        float v = 0.5 + asin(n.y) / M_PI;
        colour = texture.getColorAt(u, v);
        obj->setColour(colour);
    }

    colour = obj->lighting(lightPos, -ray.dir, ray.hit); //Object's colour


    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);

    float lightDist = glm::length(lightVec);

    if (shadowRay.index > -1 && shadowRay.dist < lightDist) {
        if (sceneObjects[shadowRay.index]->isTransparent()) colour *= obj->getTransparencyCoeff();
        else if (sceneObjects[shadowRay.index]->isRefractive()) colour *= obj->getRefractionCoeff();
        else colour *= 0.2f; //0.2 = ambient scale factor
    }

    if (obj->isTransparent() && step < MAX_STEPS) {
        float tc = obj->getTransparencyCoeff();
        Ray trancRay(ray.hit, ray.dir);
        trancRay.closestPt(sceneObjects);
        Ray trancRay2(trancRay.hit, ray.dir);
        colour += tc * trace(trancRay2, step + 1);
    }

    if (obj->isRefractive() && step < MAX_STEPS) {
        float rc = obj->getRefractionCoeff();
        float eta = 1/obj->getRefractiveIndex();
        glm::vec3 n = obj->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, n, eta);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        glm::vec3 m = obj->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta);
        Ray refracRay(refrRay.hit, h);
        colour += rc * trace(refracRay, step + 1);
    }

    if (obj->isReflective() && step < MAX_STEPS) {
        float rho = obj->getReflectionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        colour += rho * trace(reflectedRay, step + 1);
    }

    if (fog) {
        float t = (ray.hit.z + MIN_FOG)/(MIN_FOG - MAX_FOG);
        colour = (1 - t) * colour + t * glm::vec3(1, 1, 1);
    }

    return colour;
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
	glm::vec3 eye(0., 0., 0.);

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

			glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

            glm::vec3 col; //Trace the primary ray and get the colour value

            if (antiAliasing) {

                float y1 = yp + 0.25 * cellY;
                float y2 = yp + 0.75 * cellY;

                float x1 = xp + 0.25 * cellX;
                float x2 = xp + 0.75 * cellX;

                glm::vec3 dir0(x1, y1, -EDIST);
                glm::vec3 dir1(x2, y1, -EDIST);
                glm::vec3 dir2(x1, y2, -EDIST);
                glm::vec3 dir3(x2, y2, -EDIST);

                Ray ray = Ray(eye, dir0);
                col = trace(ray, 1);
                ray = Ray(eye, dir1);
                col += trace(ray, 1);
                ray = Ray(eye, dir2);
                col += trace(ray, 1);
                ray = Ray(eye, dir3);
                col += trace(ray, 1);

                col /= 4;

            } else {
                Ray ray = Ray(eye, dir);
                col = trace (ray, 1); //Trace the primary ray and get the colour value
            }


            glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);				//Draw each cell with its colour value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

    glEnd();
    glFlush();
}

void box(float x, float y, float z, float l, float w, float h, glm::vec3 colour)
{
    float xl = x + l;
    float yh = y + h;
    float zw = z - w;

    glm::vec3 V1 = glm::vec3(x, y, z);
    glm::vec3 V2 = glm::vec3(xl, y, z);
    glm::vec3 V3 = glm::vec3(xl, yh, z);
    glm::vec3 V4 = glm::vec3(x, yh, z);
    glm::vec3 V5 = glm::vec3(xl, y, zw);
    glm::vec3 V6 = glm::vec3(xl, yh, zw);
    glm::vec3 V7 = glm::vec3(x, yh, zw);
    glm::vec3 V8 = glm::vec3(x, y, zw);

    Plane *side0 = new Plane (V1, V2, V3, V4); //Front Face
    side0->setColour(colour);
    sceneObjects.push_back(side0);

    Plane *side1 = new Plane (V2, V5, V6, V3);
    side1->setColour(colour);
    sceneObjects.push_back(side1);

    Plane *side2 = new Plane (V5, V8, V7, V6);
    side2->setColour(colour);
    sceneObjects.push_back(side2);

    Plane *side3 = new Plane (V4, V7, V8, V1);
    side3->setColour(colour);
    sceneObjects.push_back(side3);

    Plane *side4 = new Plane (V4, V3, V6, V7); // Top Face
    side4->setColour(colour);
    sceneObjects.push_back(side4);

    Plane *side5 = new Plane (V8, V5, V2, V1);
    side5->setColour(colour);
    sceneObjects.push_back(side5);
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

    texture = TextureBMP("/home/cosc363/Downloads/Lab06/wall.bmp");
    earthTexture = TextureBMP("/home/cosc363/Downloads/Lab06/Earth.bmp");

    Sphere *sphere1 = new Sphere(glm::vec3(0, 5.0, -100.0), 10.0);
    sphere1->setColour(glm::vec3(0.7, 0, 1));   //Set colour to purple
    sphere1->setReflectivity(true, 0.8);
    sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

    Sphere *sphere2 = new Sphere(glm::vec3(12, -5.0, -90.0), 3);
    sceneObjects.push_back(sphere2);		 //Add sphere to scene objects

    Sphere *sphere3 = new Sphere(glm::vec3(10, -5.0, -73.0), 4.0);
    sphere3->setRefractivity(true, 1, 1.01);
    sphere3->setColour(glm::vec3(0, 0, 0));
    sceneObjects.push_back(sphere3);		 //Add sphere to scene objects

    Sphere *sphere4 = new Sphere(glm::vec3(-10.0, -5.0, -73.0), 4.0);
    sphere4->setRefractivity(true, 0.9, 1.01);
    sphere4->setColour(glm::vec3(1, 0, 0));
    sceneObjects.push_back(sphere4);		 //Add sphere to scene objects

    Plane *plane = new Plane (glm::vec3(-floorHalfWidth, -15, -40), //Point A
                             glm::vec3(floorHalfWidth, -15, -40), //Point B
                             glm::vec3(floorHalfWidth, -15, -200), //Point C
                             glm::vec3(-floorHalfWidth, -15, -200)); //Point D
    plane->setColour(glm::vec3(0.8, 0.8, 0));
    plane->setSpecularity(false);
    sceneObjects.push_back(plane);

    box(-12.5, -10, -68, 25, 10, 1, glm::vec3(0.8, 0.8, 0));

    Plane *backWall = new Plane (glm::vec3(-floorHalfWidth, -15, -200), //Point A
                             glm::vec3(floorHalfWidth, -15, -200), //Point B
                             glm::vec3(floorHalfWidth, 60, -200), //Point C
                             glm::vec3(-floorHalfWidth, 60, -200)); //Point D
    backWall->setColour(glm::vec3(0.8, 0.8, 0));
    backWall->setSpecularity(false);
    sceneObjects.push_back(backWall);

    Cone *cone1 = new Cone(glm::vec3(12.0, -18, -90.0), 5, 10);
    cone1->setColour(glm::vec3(1, 0.5, 0.1));   //Set colour to yelow
    sceneObjects.push_back(cone1);		 //Add sphere to scene objects

    Cylinder *cylinder1 = new Cylinder(glm::vec3(0.0, -8, -73.0), 2, 3);
    cylinder1->setColour(glm::vec3(1, 0.1, 0.1));   //Set colour to yelow
    sceneObjects.push_back(cylinder1);		 //Add sphere to scene objects

    Cylinder *cylinder2 = new Cylinder(glm::vec3(0.0, -15, -73.0), 3, 5);
    cylinder2->setColour(glm::vec3(1, 0.1, 0.1));   //Set colour to yelow
    sceneObjects.push_back(cylinder2);		 //Add sphere to scene objects

    Cone *cone2 = new Cone(glm::vec3(-12.0, -18, -90.0), 5, 10);
    cone2->setColour(glm::vec3(1, 0.5, 0.6));   //Set colour to yelow
    sceneObjects.push_back(cone2);		 //Add sphere to scene objects

    Sphere *sphere5 = new Sphere(glm::vec3(-12, -5.0, -90.0), 3);
    sceneObjects.push_back(sphere5);		 //Add sphere to scene objects

    Sphere *sphere6 = new Sphere(glm::vec3(-5, -8.0, -60.0), 5);
    sphere6->setTransparency(true, 0.9);
    sphere6->setColour(glm::vec3(0, 0, 0));
    sceneObjects.push_back(sphere6);		 //Add sphere to scene objects




}

void extraFeature(unsigned char key, int x, int y)
{
    if(key == 'a')
        antiAliasing = !antiAliasing;
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");
    initialize();
    glutDisplayFunc(display);
    glutKeyboardFunc(extraFeature);

    glutMainLoop();
    return 0;
}
