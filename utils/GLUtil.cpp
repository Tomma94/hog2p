/*
 *  $Id: glUtil.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 6/8/05.
 *  Modified by Nathan Sturtevant on 02/29/20.
 *
 * This file is part of HOG2. See https://github.com/nathansttt/hog2 for licensing information.
 *
 */

#include "GLUtil.h"
#include "FPUtil.h"
#include <math.h>
#include <assert.h>
#include <vector>
#include <cstring>

bool fastCrossTest(float p0_x, float p0_y, float p1_x, float p1_y, 
				   float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y);

bool operator==(const recVec &l1, const recVec &l2)
{
	return (fequal(l1.x, l2.x) && fequal(l1.y, l2.y));
}

std::ostream& operator <<(std::ostream &out, const recVec &loc)
{
	out << "(" << loc.x << ", " << loc.y << ", " << loc.z <<")";
	return out;
}
	

/**
* Normalize a vector.
 *
 * this really should be part of the recVec class -- normalizes a vector
 */
void recVec::normalise()
{
	double length = this->length();

	if (length != 0)
	{
		x /= length;
		y /= length;
		z /= length;
	}
	else {
		x = 0;
		y = 0;
		z = 0;
	}
}

double recVec::length() const
{
	return sqrt(x * x + y * y + z * z);
}

bool line2d::crosses(line2d which) const
{
	if ((which.start == start) || (which.end == end) ||
		(which.start == end) || (which.end == start))
		return false;

	//	//input x1,y1 input x2,y2
	//input u1,v1 input u2,v2
	line2d here(start, end);
	double maxx1, maxx2, maxy1, maxy2;
	double minx1, minx2, miny1, miny2;
	if (here.start.x > here.end.x)
	{ maxx1 = here.start.x; minx1 = here.end.x; }
	else
	{ maxx1 = here.end.x; minx1 = here.start.x; }
	
	if (here.start.y > here.end.y)
	{ maxy1 = here.start.y; miny1 = here.end.y; }
	else
	{ maxy1 = here.end.y; miny1 = here.start.y; }
	
	if (which.start.x > which.end.x)
	{ maxx2 = which.start.x; minx2 = which.end.x; }
	else
	{ maxx2 = which.end.x; minx2 = which.start.x; }
	
	if (which.start.y > which.end.y)
	{ maxy2 = which.start.y; miny2 = which.end.y; }
	else
	{ maxy2 = which.end.y; miny2 = which.start.y; }
	
	if (fless(maxx1,minx2) || fless(maxx2, minx1) || fless(maxy1, miny2) || fless(maxy2, miny1))
		return false;

	return fastCrossTest(start.x, start.y, end.x, end.y, 
						 which.start.x, which.start.y, which.end.x, which.end.y, 
						 0, 0);
	//	
	if (fequal(maxx1, minx1)) // this is "here"
	{
		// already know that they share bounding boxes
		// here, they must cross
		if ((maxy2 < maxy1) && (miny2 > miny1))
			return true;
		
		// y = mx + b
		double m = (which.end.y-which.start.y)/(which.end.x-which.start.x);
		double b = which.start.y - m*which.start.x;
		double y = m*here.start.x+b;
		if (fless(y, maxy1) && fgreater(y, miny1)) // on the line
			return true;
		return false;
	}
	if (fequal(maxx2, minx2)) // this is "which"
	{
		// already know that they share bounding boxes
		// here, they must cross
		if ((maxy1 < maxy2) && (miny1 > miny2))
			return true;
		
		// y = mx + b
		double m = (here.end.y-here.start.y)/(here.end.x-here.start.x);
		double b = here.start.y - m*here.start.x;
		double y = m*which.start.x+b;
		if (fless(y, maxy2) && fgreater(y, miny2)) // on the line
			return true;
		return false;
	}
	
	double b1 = (which.end.y-which.start.y)/(which.end.x-which.start.x);// (A)
	double b2 = (here.end.y-here.start.y)/(here.end.x-here.start.x);// (B)
	
	double a1 = which.start.y-b1*which.start.x;
	double a2 = here.start.y-b2*here.start.x;
	
	if (fequal(b1, b2))
		return false;
	double xi = - (a1-a2)/(b1-b2); //(C)
	double yi = a1+b1*xi;
	// these are actual >= but we exempt points
	if ((!fless((which.start.x-xi)*(xi-which.end.x), 0)) &&
		(!fless((here.start.x-xi)*(xi-here.end.x), 0)) &&
		(!fless((which.start.y-yi)*(yi-which.end.y), 0)) &&
		(!fless((here.start.y-yi)*(yi-here.end.y), 0)))
	{
		//printf("lines cross at (%f, %f)\n",xi,yi);
		return true;
	}
	else {
		return false;
		//print "lines do not cross";
	}
	assert(false);
}

// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines 
// intersect the intersection point may be stored in the floats i_x and i_y.
// taken from: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
//
bool fastCrossTest(float p0_x, float p0_y, float p1_x, float p1_y, 
				   float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;
	
    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);
	
    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
//        if (i_x != NULL)
//            *i_x = p0_x + (t * s1_x);
//        if (i_y != NULL)
//            *i_y = p0_y + (t * s1_y);
        return 1;
    }
	
    return 0; // No collision
}


void DrawPyramid(GLfloat x, GLfloat y, GLfloat z, GLfloat height, GLfloat width)
{
	glBegin(GL_TRIANGLES);
	//	glNormal3f(ROOT2D2, -ROOT2D2, 0);
	glNormal3f(-ROOT2D2, ROOT2D2, 0);
	glVertex3f(x, y, z-height);
	glVertex3f(x-width, y-width, z);
	glVertex3f(x-width, y+width, z);

	glNormal3f(0, ROOT2D2, ROOT2D2);
	glVertex3f(x, y, z-height);
	glVertex3f(x-width, y+width, z);
	glVertex3f(x+width, y+width, z);

	glNormal3f(ROOT2D2, ROOT2D2, 0);
	glVertex3f(x, y, z-height);
	glVertex3f(x+width, y+width, z);
	glVertex3f(x+width, y-width, z);

	glNormal3f(0, ROOT2D2, -ROOT2D2);
	glVertex3f(x, y, z-height);
	glVertex3f(x+width, y-width, z);
	glVertex3f(x-width, y-width, z);
	glEnd();
}

// interleaved vertex array for glDrawElements() & glDrawRangeElements() ======
// All vertex attributes (position, normal, color) are packed together as a
// struct or set, for example, ((V,N,C), (V,N,C), (V,N,C),...).
// It is called an array of struct, and provides better memory locality.
GLfloat vertices3[] = { 1, 1, 1,   0, 0, 1,   1, 1, 1,              // v0 (front)
	-1, 1, 1,   0, 0, 1,   1, 1, 0,              // v1
	-1,-1, 1,   0, 0, 1,   1, 0, 0,              // v2
	1,-1, 1,   0, 0, 1,   1, 0, 1,              // v3
	
	1, 1, 1,   1, 0, 0,   1, 1, 1,              // v0 (right)
	1,-1, 1,   1, 0, 0,   1, 0, 1,              // v3
	1,-1,-1,   1, 0, 0,   0, 0, 1,              // v4
	1, 1,-1,   1, 0, 0,   0, 1, 1,              // v5
	
	1, 1, 1,   0, 1, 0,   1, 1, 1,              // v0 (top)
	1, 1,-1,   0, 1, 0,   0, 1, 1,              // v5
	-1, 1,-1,   0, 1, 0,   0, 1, 0,              // v6
	-1, 1, 1,   0, 1, 0,   1, 1, 0,              // v1
	
	-1, 1, 1,  -1, 0, 0,   1, 1, 0,              // v1 (left)
	-1, 1,-1,  -1, 0, 0,   0, 1, 0,              // v6
	-1,-1,-1,  -1, 0, 0,   0, 0, 0,              // v7
	-1,-1, 1,  -1, 0, 0,   1, 0, 0,              // v2
	
	-1,-1,-1,   0,-1, 0,   0, 0, 0,              // v7 (bottom)
	1,-1,-1,   0,-1, 0,   0, 0, 1,              // v4
	1,-1, 1,   0,-1, 0,   1, 0, 1,              // v3
	-1,-1, 1,   0,-1, 0,   1, 0, 0,              // v2
	
	1,-1,-1,   0, 0,-1,   0, 0, 1,              // v4 (back)
	-1,-1,-1,   0, 0,-1,   0, 0, 0,              // v7
	-1, 1,-1,   0, 0,-1,   0, 1, 0,              // v6
	1, 1,-1,   0, 0,-1,   0, 1, 1 };            // v5

GLubyte indices[]  = { 0, 1, 2,   2, 3, 0,      // front
	4, 5, 6,   6, 7, 4,      // right
	8, 9,10,  10,11, 8,      // top
	12,13,14,  14,15,12,      // left
	16,17,18,  18,19,16,      // bottom
	20,21,22,  22,23,20 };    // back

void DrawBoxFrame(GLfloat xx, GLfloat yy, GLfloat zz, GLfloat rad)
{
	GLfloat vertices[] =
	{-1, -1, -1, -1, -1,  1,
	 -1, -1, -1, -1,  1, -1,
	 -1, -1, -1,  1, -1, -1,
	  1,  1,  1,  1,  1, -1,
	  1,  1,  1,  1, -1,  1,
	  1,  1,  1, -1,  1,  1,
	  1,  1, -1, -1,  1, -1,
	  1,  1, -1,  1, -1, -1,
	 -1,  1,  1, -1, -1,  1,
	 -1,  1,  1, -1,  1, -1,
	  1, -1, -1,  1, -1,  1,
	 -1, -1,  1,  1, -1,  1
	 }; // 36 of vertex coords

	// activate and specify pointer to vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	
	glPushMatrix();
	glTranslatef(xx, yy, zz);                // move to bottom-left
	glScalef(rad, rad, rad);
	glDrawArrays(GL_LINES, 0, 24);
	
	glPopMatrix();
	
	// deactivate vertex arrays after drawing
	glDisableClientState(GL_VERTEX_ARRAY);

}

void DrawBox(GLfloat xx, GLfloat yy, GLfloat zz, GLfloat rad)
{
	glEnable(GL_NORMALIZE);
	// enable and specify pointers to vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glNormalPointer(GL_FLOAT, 9 * sizeof(GLfloat), vertices3 + 3);
	//glColorPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), vertices3 + 6);
	glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), vertices3);
	
	glPushMatrix();
	glTranslatef(xx, yy, zz);                // move to bottom-left
	glScalef(rad, rad, rad);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);
	
	glPopMatrix();
	
	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
	//glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_NORMALIZE);
}

void DrawCylinder(GLfloat xx, GLfloat yy, GLfloat zz, GLfloat innerRad, GLfloat outerRad, GLfloat height)
{
	const double resolution = 30;
	const double step = TWOPI/resolution;
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, 1, 0);
	for (double t = 0; t < TWOPI; t += step)
	{
		glVertex3f(xx+innerRad*cos(t), yy-height/2, zz+innerRad*sin(t));
		glVertex3f(xx+outerRad*cos(t), yy-height/2, zz+outerRad*sin(t));
	}
	glVertex3f(xx+innerRad*cos(0), yy-height/2, zz+innerRad*sin(0));
	glVertex3f(xx+outerRad*cos(0), yy-height/2, zz+outerRad*sin(0));
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, -1, 0);
	for (double t = 0; t < TWOPI; t += step)
	{
		glVertex3f(xx+innerRad*cos(t), yy+height/2, zz+innerRad*sin(t));
		glVertex3f(xx+outerRad*cos(t), yy+height/2, zz+outerRad*sin(t));
	}
	glVertex3f(xx+innerRad*cos(0), yy+height/2, zz+innerRad*sin(0));
	glVertex3f(xx+outerRad*cos(0), yy+height/2, zz+outerRad*sin(0));
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	for (double t = 0; t < TWOPI; t += step)
	{
		glNormal3f(-cos(t), 0, -sin(t));
		glVertex3f(xx+outerRad*cos(t), yy+height/2, zz+outerRad*sin(t));
		glVertex3f(xx+outerRad*cos(t), yy-height/2, zz+outerRad*sin(t));
	}
	glVertex3f(xx+outerRad*cos(0), yy+height/2, zz+outerRad*sin(0));
	glVertex3f(xx+outerRad*cos(0), yy-height/2, zz+outerRad*sin(0));
	glEnd();

}

//
//void DrawBox(GLfloat xx, GLfloat yy, GLfloat zz, GLfloat rad)
//{
//	glBegin(GL_QUAD_STRIP);
//	glVertex3f(xx-rad, yy-rad, zz-rad);
//	glVertex3f(xx-rad, yy+rad, zz-rad);
//
//	glVertex3f(xx+rad, yy-rad, zz-rad);
//	glVertex3f(xx+rad, yy+rad, zz-rad);
//
//	glVertex3f(xx+rad, yy-rad, zz+rad);
//	glVertex3f(xx+rad, yy+rad, zz+rad);
//
//	glVertex3f(xx-rad, yy-rad, zz+rad);
//	glVertex3f(xx-rad, yy+rad, zz+rad);
//
//	glVertex3f(xx-rad, yy-rad, zz-rad);
//	glVertex3f(xx-rad, yy+rad, zz-rad);
//
//	glEnd();
//
//	glBegin(GL_QUADS);
//	glVertex3f(xx-rad, yy+rad, zz-rad);
//	glVertex3f(xx+rad, yy+rad, zz-rad);
//	glVertex3f(xx+rad, yy+rad, zz+rad);
//	glVertex3f(xx-rad, yy+rad, zz+rad);
//	glEnd();
//
//	glBegin(GL_QUADS);
//	glVertex3f(xx-rad, yy-rad, zz-rad);
//	glVertex3f(xx+rad, yy-rad, zz-rad);
//	glVertex3f(xx+rad, yy-rad, zz+rad);
//	glVertex3f(xx-rad, yy-rad, zz+rad);
//	glEnd();
//}

void OutlineRect(GLdouble left, GLdouble top, GLdouble right, GLdouble bottom, double zz)
{
	glDisable(GL_LIGHTING);
	glBegin(GL_LINE_LOOP);
	glVertex3f(left, top, zz);
	glVertex3f(right, top, zz);
	glVertex3f(right, bottom, zz);
	glVertex3f(left, bottom, zz);
	glEnd();
	
}

void DrawSquare(GLdouble xx, GLdouble yy, GLdouble zz, GLdouble rad)
{
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(xx-rad, yy-rad, zz);
	glVertex3f(xx+rad, yy-rad, zz);
	glVertex3f(xx-rad, yy+rad, zz);
	glVertex3f(xx+rad, yy+rad, zz);
	glEnd();

}

void FrameCircle(GLdouble _x, GLdouble _y, GLdouble tRadius, GLdouble lineWidth, int segments, float rotation)
{
	double resolution = TWOPI/segments;
	glBegin(GL_TRIANGLE_STRIP);
	for (int x = 0; x <= segments; x++)
	{
		GLdouble s = sin(resolution*x+rotation*TWOPI/360.0);
		GLdouble c = cos(resolution*x+rotation*TWOPI/360.0);
		glVertex2f(_x+s*(tRadius-lineWidth/2), _y+c*(tRadius-lineWidth/2));
		glVertex2f(_x+s*(tRadius+lineWidth/2), _y+c*(tRadius+lineWidth/2));
	}
	glEnd();
}

void DrawCircle(GLdouble _x, GLdouble _y, GLdouble tRadius, int segments, float rotation)
{
	double resolution = TWOPI/segments;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(_x, _y);
	for (int x = 0; x <= segments; x++)
	{
		glVertex2f(_x+sin(resolution*x+rotation*TWOPI/360.0)*tRadius, _y+cos(resolution*x+rotation*TWOPI/360.0)*tRadius);
	}
	glEnd();
}

void DrawSphere(GLdouble _x, GLdouble _y, GLdouble _z, GLdouble tRadius)
{
	static std::vector<double> px_cache;
	static std::vector<double> py_cache;
	static std::vector<double> pz_cache;
	glEnable(GL_LIGHTING);
	
	glTranslatef(_x, _y, _z);

	int i,j;
	int n = 16; // precision
	double theta1,theta2,theta3;
	point3d e,p;//,c(0, 0, 0);
	
	if (tRadius < 0) tRadius = -tRadius;
	if (n < 0) n = -n;
	if (n < 4 || tRadius <= 0)
	{
		glBegin(GL_POINTS);
		glVertex3f(0, 0, 0);
		glEnd();
	}
	else if (px_cache.size() == 0)
	{
		for (j=n/4;j<n/2;j++)
		{
			theta1 = j * TWOPI / n - PID2;
			theta2 = (j + 1) * TWOPI / n - PID2;
			
			glBegin(GL_QUAD_STRIP);
			//glBegin(GL_POINTS);
			//glBegin(GL_TRIANGLE_STRIP);
			//glBegin(GL_LINE_STRIP);
			for (i=0;i<=n;i++)
			{
				theta3 = i * TWOPI / n;
				
				e.x = cos(theta2) * cos(theta3);
				e.y = cos(theta2) * sin(theta3);
				e.z = sin(theta2);

				px_cache.push_back(e.x);
				py_cache.push_back(e.y);
				pz_cache.push_back(e.z);
				
				p.x = tRadius * e.x;
				p.y = tRadius * e.y;
				p.z = - tRadius * e.z;
				
				glNormal3f(-e.x,-e.y,e.z);
				//glTexCoord2f(i/(double)n,2*(j+1)/(double)n);
				glVertex3f(p.x,p.y,p.z);
				
				e.x = cos(theta1) * cos(theta3);
				e.y = cos(theta1) * sin(theta3);
				e.z = sin(theta1);

				px_cache.push_back(e.x);
				py_cache.push_back(e.y);
				pz_cache.push_back(e.z);

				p.x = tRadius * e.x;
				p.y = tRadius * e.y;
				p.z = - tRadius * e.z;
				
				glNormal3f(-e.x,-e.y,e.z);
				//glTexCoord2f(i/(double)n,2*j/(double)n);
				glVertex3f(p.x,p.y,p.z);
			}
			glEnd();
		}
	}
	else {
		int which = 0;
		for (j=n/4;j<n/2;j++)
		{
			glBegin(GL_QUAD_STRIP);
			for (i=0;i<=n;i++)
			{
				glNormal3d(-px_cache[which], -py_cache[which], pz_cache[which]);
				glVertex3d(tRadius*px_cache[which], tRadius*py_cache[which], -tRadius*pz_cache[which]);
				which++;
				glNormal3d(-px_cache[which], -py_cache[which], pz_cache[which]);
				glVertex3d(tRadius*px_cache[which], tRadius*py_cache[which], -tRadius*pz_cache[which]);
				which++;
			}
			glEnd();
		}
	}
	glTranslatef(-_x, -_y, -_z);
	glDisable(GL_LIGHTING);
}	

void DrawText(double x, double y, double z, double scale, const char *str)
{
	glPushMatrix();
	//glEnable(GL_LINE_SMOOTH);
	glTranslatef(x, y, z);
	glScalef(scale/300, scale/300.0, 1);
	glRotatef(180, 0.0, 0.0, 1.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glDisable(GL_LIGHTING);
	// for (size_t which = 0; which < strlen(str); which++)
	// 	glutStrokeCharacter(GLUT_STROKE_ROMAN, str[which]);
//	glEnable(GL_LIGHTING);
	//glTranslatef(-x/width+0.5, -y/height+0.5, 0);
	glPopMatrix();
}

void DrawTextCentered(double x, double y, double z, double scale, const char *str)
{
	glPushMatrix();
	
	int width = 0;
	// for (size_t which = 0; which < strlen(str); which++)
	// 	width += glutStrokeWidth(GLUT_STROKE_ROMAN, str[which]);
	
	glTranslatef(x, y, z);
	glScalef(scale/300, scale/300.0, 1);
	glRotatef(180, 0.0, 0.0, 1.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glDisable(GL_LIGHTING);
	glTranslatef(-width/2, -4.0/scale, 0);
	
	// for (size_t which = 0; which < strlen(str); which++)
	// 	glutStrokeCharacter(GLUT_STROKE_ROMAN, str[which]);

//	glEnable(GL_LIGHTING);
	//glTranslatef(-x/width+0.5, -y/height+0.5, 0);
	glPopMatrix();
}

void SetLighting(GLfloat ambientf, GLfloat diffusef, GLfloat specularf)
{
	//	GLfloat mat_specular[] = {0.2, 0.2, 0.2, 1.0};
	//	GLfloat mat_shininess[] = {50.0};
	
	//	GLfloat position[4] = {7.0,-7.0,12.0,0.0};
	//	GLfloat position[4] = {-1.0,-3.0,5.0,0.0};
	//	GLfloat position[4] = {-1.0,5.0,5.0,0.0};
	//	GLfloat position[4] = {-0.0,1.0,3.0,0.0};
	GLfloat position[4] = {-5.0,5.0,30.0,0.0};
	GLfloat ambient[4]  = {1.0, 1.0, 1.0, 1.0};
	GLfloat diffuse[4]  = {1.0, 1.0, 1.0, 1.0};
	GLfloat specular[4] = {1.0, 1.0, 1.0, 1.0};
	for (int x = 0; x < 3; x++)
	{
		ambient[x] *= ambientf;
		diffuse[x] *= diffusef;
		specular[x] *= specularf;
	}
	
	//	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	//	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	
	//	switch (mode) {
	//		case 0:
	//			break;
	//		case 1:
	//			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	//			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
	//			break;
	//		case 2:
	//			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	//			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	//			break;
	//		case 3:
	//			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	//			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
	//			break;
	//		case 4:
	//			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	//			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	//			break;
	//	}
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	
	
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glEnable(GL_LIGHT0);
}

