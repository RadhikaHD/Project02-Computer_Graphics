/*
 * Alekhya Kamana, Radhika Dhaipule, & Christian Stith
 * CPSC 6050: Introduction to Graphics
 * Dr. Robert Geist
 * March 3, 2015
 * Project 2: Show Me the Bunny!
 * 
 * Compilation:
 * 		gcc bunny.c -lGL -lGLU -lglut -o bunny.out
 * Execution:
 * 		./bunny.out
 * This project displays a basic rendered version of the Stanford bunny
 * and implements Normalized Blinn-Phong Shading, Motion Blur, Anti-Aliasing,
 * Three-Point Lighting, Camera Movement, and VBOs. See README for more information.
 */


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

#define VPASSES 150
#define EMOVE 0.02
#define ADDEACH 0.02
#define FADE 0.98
#define MAX_LENGTH 2000000


struct point {
	float x, y, z;
};

struct face {
	long v[3], t[3], n[3];
};

float cameraX=-3.0f, cameraY=3.0, cameraZ=5.0f;

int mybuf=1;

int blur = 0;

long numVertices = 0;
struct point vertexArray[MAX_LENGTH];
long numNormals = 0;
struct point normalArray[MAX_LENGTH];
long numFaces = 0;
struct face faceArray[MAX_LENGTH];

GLfloat indices[MAX_LENGTH];

void viewvolume_shape() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,1.0,0.1,20.0);
}

struct point cross(struct point u, struct point v) {
	struct point w;
	w.x = u.y*v.z - u.z*v.y;
	w.y = -(u.x*v.z - u.z*v.x);
	w.z = u.x*v.y - u.y*v.x;
	return(w);
}

struct point unit_length(struct point u) {
	double length;
	struct point v;
	length = sqrt(u.x*u.x+u.y*u.y+u.z*u.z);
	v.x = u.x/length;
	v.y = u.y/length;
	v.z = u.z/length;
	return(v);
}

void load_vector( FILE* ifp, struct point array[], long* counter ) {
	fscanf(ifp, "%f", &array[*counter].x);
	fscanf(ifp, "%f", &array[*counter].y);
	fscanf(ifp, "%f", &array[*counter].z);
}

void load_vector_face( FILE* ifp, struct face array[], long* counter ) {
	int i;
	char slash;
	for ( i=0; i<3; i++ ) {
		fscanf(ifp, "%ld", &array[*counter].v[i]);
		fscanf(ifp, "%c", &slash);
		fscanf(ifp, "%ld", &array[*counter].t[i]);
		fscanf(ifp, "%c", &slash);
		fscanf(ifp, "%ld", &array[*counter].n[i]);
	}
}

void load_obj(char* fname) {
	
	FILE *ifp;
	ifp = fopen(fname, "r");
	int i;
	char lineType[2];
	while ( fscanf(ifp, "%s", lineType) != EOF ) {
		if ( lineType[0] == 'v' ) {
			if ( lineType[1] == 'n' ) {
				load_vector(ifp, normalArray, &numNormals);
				numNormals++;
			} else {
				load_vector(ifp, vertexArray, &numVertices);
				numVertices++;
			}
		} else if ( lineType[0] == 'f' ) {
			load_vector_face(ifp, faceArray, &numFaces);
			numFaces++;
		}
		char line[60];
		fgets (line, 60, ifp);
	}
	
	fclose(ifp);
	
	for( i=0; i<numFaces; i++ ) {
		int j = 0;
		for (j=0; j<3; j++) {
			indices[i*9 + j*3 + 0] = vertexArray[faceArray[i].v[j]-1].x;
			indices[i*9 + j*3 + 1] = vertexArray[faceArray[i].v[j]-1].y;
			indices[i*9 + j*3 + 2] = vertexArray[faceArray[i].v[j]-1].z;
			indices[numFaces*9 + i*9 + j*3 + 0] = normalArray[faceArray[i].n[j]-1].x;
			indices[numFaces*9 + i*9 + j*3 + 1] = normalArray[faceArray[i].n[j]-1].y;
			indices[numFaces*9 + i*9 + j*3 + 2] = normalArray[faceArray[i].n[j]-1].z;
		}	
	}	
}

void set_camera() {

	struct point lookAt = { -0.5, 1.0, 0.0 };
	struct point viewDirection, vtemp, utemp;
	struct point eye = {cameraX, cameraY, cameraZ}, up = {0.0, 1.0, 0.0};
	
	viewDirection.x = lookAt.x - eye.x;
	viewDirection.y = lookAt.y - eye.y;
	viewDirection.z = lookAt.z - eye.z;
	vtemp = cross(viewDirection,up);
	utemp = cross(vtemp,viewDirection);
	up.x = unit_length(utemp).x;
	up.y = unit_length(utemp).y;
	up.z = unit_length(utemp).z;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z);
}

void draw_array() {
	glClearColor(0,0,0,0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, numFaces*3);
}

void set_lights() {
	
	//key
	float light0_ambient[] = { 139.0/255.0, 69.0/255.0, 19.0/255.0, 0.2 };
	float light0_diffuse[] = {0.6, 1.0, 0.6, 1.0}; 
	float light0_specular[] = { 2.0, 2.0, 2.0, 0.0 }; 
	float light0_position[] = { 12.0, 12.0, 36.0, 1.0 };
	float light0_direction[] = { -2.0, -2.0, 6.0, 1.0};
	
	//fill
	float light1_ambient[] = { 139.0/255.0, 69.0/255.0, 19.0/255.0, 0.2 };
	float light1_diffuse[] = {0.4, 0.4, 0.8, 1.0}; 
	float light1_specular[] = { 2.25, 2.25, 2.25, 0.0 }; 
	float light1_position[] = { -12.0, 8.0, 36.0, 1.0 };
	float light1_direction[] = { 2.0, -2.0, 4.0, 1.0};

	//back
	float light2_ambient[] = { 139.0/255.0, 69.0/255.0, 19.0/255.0, 0.2 };
	float light2_diffuse[] = {1.0, 0.0, 0.0, 1.0}; 
	float light2_specular[] = { 2.25, 2.25, 2.25, 0.0 }; 
	float light2_position[] = { 0.0, 4.0, -36.0, 1.0 };
	float light2_direction[] = { 0.0, -2.0, -4.0, 1.0};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,light0_ambient); 
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1); 
	glLightfv(GL_LIGHT0,GL_AMBIENT,light0_ambient); 
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light0_diffuse); 
	glLightfv(GL_LIGHT0,GL_SPECULAR,light0_specular); 
	glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,1.0); 
	glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,180.0); 
	glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0); 
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.2); 
	glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.01); 
	glLightfv(GL_LIGHT0,GL_POSITION,light0_position);
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,light0_direction);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glLightfv(GL_LIGHT1,GL_AMBIENT,light1_ambient); 
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light1_diffuse); 
	glLightfv(GL_LIGHT1,GL_SPECULAR,light1_specular); 
	glLightf(GL_LIGHT1,GL_SPOT_EXPONENT,1.0); 
	glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,180.0); 
	glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION,1.0); 
	glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,0.2); 
	glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION,0.01); 
	glLightfv(GL_LIGHT1,GL_POSITION,light1_position);
	glLightfv(GL_LIGHT1,GL_SPOT_DIRECTION,light1_direction);
	glEnable(GL_LIGHT1);
	
		
	glLightfv(GL_LIGHT2,GL_AMBIENT,light2_ambient); 
	glLightfv(GL_LIGHT2,GL_DIFFUSE,light2_diffuse); 
	glLightfv(GL_LIGHT2,GL_SPECULAR,light2_specular); 
	glLightf(GL_LIGHT2,GL_SPOT_EXPONENT,1.0); 
	glLightf(GL_LIGHT2,GL_SPOT_CUTOFF,180.0); 
	glLightf(GL_LIGHT2,GL_CONSTANT_ATTENUATION,1.0); 
	glLightf(GL_LIGHT2,GL_LINEAR_ATTENUATION,0.2); 
	glLightf(GL_LIGHT2,GL_QUADRATIC_ATTENUATION,0.01); 
	glLightfv(GL_LIGHT2,GL_POSITION,light2_position);
	glLightfv(GL_LIGHT2,GL_SPOT_DIRECTION,light2_direction);
	glEnable(GL_LIGHT2);
	
}

char *read_shader_program(char *filename)  {
	FILE *fp;
	char *content = NULL;
	int fd, count;
	fd = open(filename,O_RDONLY);
	count = lseek(fd,0,SEEK_END);
	close(fd);
	content = (char *)calloc(1,(count+1));
	fp = fopen(filename,"r");
	count = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
	fclose(fp);
	return content;
}

void set_material() {
	float mat_ambient[] = {0.1,0.18725,0.1745,1.0}; 
	float mat_diffuse[] = {0.396,0.74151,0.9102,1.0}; 
	float mat_specular[] = {0.297254,0.30829,0.306678,1.0};
	float mat_shininess[] = {70.8};
	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
}

void blur_display() {
	int view_pass;
	glClear(GL_ACCUM_BUFFER_BIT);
	glTranslatef(-2.0, 0.0, 2.0);
	draw_array();
	glFlush();
	glAccum(GL_ACCUM, ADDEACH);
	for( view_pass=0; view_pass<VPASSES; view_pass++ ){
		glAccum(GL_MULT, FADE);
		glTranslatef(EMOVE, 0, -EMOVE);
		draw_array();
		glFlush();
		glAccum(GL_ACCUM, ADDEACH);
	}
	set_material();
	draw_array();
	glAccum(GL_ACCUM, 20.0*ADDEACH);
	glAccum(GL_RETURN,1.0);
	glFlush();
	glutSwapBuffers();
}

void normal_display() {
	set_camera();
	draw_array();
	glutSwapBuffers();
}

unsigned int set_shaders() {
	GLint vertCompiled, fragCompiled;
	char *vs, *fs;
	GLuint v, f, p;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	vs = read_shader_program("phong.vert");
	fs = read_shader_program("phong.frag");
	glShaderSource(v,1,(const char **)&vs,NULL);
	glShaderSource(f,1,(const char **)&fs,NULL);
	free(vs);
	free(fs); 
	glCompileShader(v);
	glCompileShader(f);
	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,v);
	glLinkProgram(p);
	glUseProgram(p);
	return(p);
}

void input(unsigned char key, int xx, int yy) {

	float delta = 1.0f;
	
	switch (key) {
		case 'x':
			glDeleteBuffers(1,&mybuf);
			exit(1);
		case 'a' :
			cameraX -= delta;
			break;
		case 'd' :
			cameraX += delta;
			break;
		case 'w' :
			cameraY += delta;
			break;
		case 's' :
			cameraY -= delta;
			break;
		case 'q' :
			cameraZ += delta;
			break;
		case 'e' :
			cameraZ -= delta;
			break;
		case ' ':
			if ( blur ) {
				glutDisplayFunc(normal_display);
				blur = 0;
			}
			else {
				glutDisplayFunc(blur_display);			
				blur = 1;
			}
			break;
	}
	set_camera();
	glutPostRedisplay();
}


int main(int argc, char **argv)  {
	unsigned int p;
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_MULTISAMPLE);
	glutInitWindowSize(720,720);
	glutInitWindowPosition(100,50);
	glutCreateWindow("Kamana Dhaipule Stith");
	glutKeyboardFunc(input);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE_ARB);
	viewvolume_shape();
	
	set_lights();
	set_material();
	set_camera();
	load_obj("bunny.obj");

	glBindBuffer(GL_ARRAY_BUFFER, mybuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 3*sizeof(GLfloat), NULL+0);
	glNormalPointer(GL_FLOAT, 3*sizeof(GLfloat), NULL+3*numFaces*3*sizeof(GLfloat));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	p = set_shaders();
	glutDisplayFunc(normal_display);
	
	glutMainLoop();
	return 0;
}
