#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdio.h>
#include <unistd.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <png++/png.hpp>

#include "meshDef.cpp"
// Shader sources

const GLchar* vshader =
    "#version 150 \n"
    "in vec3 position;"
    "uniform mat4 transform;"
	"void main()"
    "{"
    "    gl_Position  = transform*vec4(position, 1.0);"
    "}";

const GLchar* fshader =
    "#version 150 \n"
    "uniform vec4 lineColor;"
    "out vec4 fragColor;"
    "void main()"
    "{"
    "    fragColor = lineColor;"
    "}";

//Global Variables
GLuint vao, vbo,shaderProgram ;
GLfloat  xscale, yscale, zscale;
uint8_t *pixels = new uint8_t[800 * 600 * 3];

int showSlice(slice *s, float &, float &, float &);
int max_slice_no=0,cur_slice_no=0, vertexCount=0;
int width=0, height=0;

int endPointCount=0;
//functions
float minimum (GLfloat x, GLfloat y, GLfloat z) {
	float min = x; /* assume x is the largest */
	if (y < min) 
		min = y;
	if (z < min ) 
		min = z;

	return min ; /* max is the largest value */
} 

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window,GL_TRUE);
	
//	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && cur_slice_no<max_slice_no)

}


//R(ange)scale_x, y and z
GLFWwindow* glInit(int slicecounter, const GLfloat Rscale_x, const GLfloat Rscale_y, const GLfloat Rscale_z, const GLfloat pixels_per_mm = 0.5) {
	//store globally relevant info
	max_slice_no=slicecounter;

	xscale = (pixels_per_mm)/(2*Rscale_x);
	yscale = (pixels_per_mm)/(2*Rscale_y);
	zscale = 1/(2*Rscale_z);

	GLFWwindow* window;
	if (!glfwInit())
	    exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	window = glfwCreateWindow(800, 600, "Slice Viewer", NULL, NULL);
if (!window) {
	    glfwTerminate();
	    exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSwapInterval(1);

	glfwGetFramebufferSize (window, &width, &height);

	glewExperimental = GL_TRUE;
	if(glewInit()!=GLEW_OK) {
		cout<<"\n(Diagnostic Msg) Error: Couldn't initiazlie glew.";
		exit(EXIT_FAILURE);
	}

	glGetError();
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource (vertexShader, 1, &vshader, NULL);
	glCompileShader(vertexShader);

	//check vertex shader compilation
	GLint status=0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE) {
		GLint logLen=0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLen);
		GLchar* logStr = new GLchar[logLen];
		glGetShaderInfoLog(vertexShader, logLen, 0, logStr);

		cout<<"\nVertex Shader Error "<<logStr;
	}
	
	//frag shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);	
	glShaderSource (fragmentShader, 1, &fshader, NULL);	
	glCompileShader(fragmentShader);
	status=0;
	//check fragshader compilation
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE) {
		GLint logLen=0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLen);
		GLchar* logStr = new GLchar[logLen];
		glGetShaderInfoLog(fragmentShader, logLen, 0, logStr);

		cout<<"\nFragment Shader Error "<<logStr;
	}
	
	//shader program
	shaderProgram = glCreateProgram();
	
	glAttachShader(shaderProgram, fragmentShader);
	glAttachShader( shaderProgram, vertexShader);

	glLinkProgram(shaderProgram);

	status=0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	
	//check linking
	if(status==GL_FALSE) {
		GLint logLen=0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);
		cout<<"\nLog Length: "<<logLen<<"\n";

		GLchar* logStr = new GLchar[logLen];
		glGetProgramInfoLog(shaderProgram, logLen, 0, logStr);

		cout<<"\nProgram Linking Error "<<logStr;
	}
	
	glUseProgram(shaderProgram);
	
	return window;
}

int boundaryVertexCount=0;


bool xCoordinateComparision(const glm::vec3 &a, const glm::vec3 &b){

	return a.x<b.x;
}

vector <glm::vec3>  lineFill(vector <glm::vec3> vertices) { 

	if(boundaryVertexCount == 0)
		return vertices;

	vector <glm::vec3> addedVertices, intersections;

	float yMin = vertices[0].y, yMax = vertices[0].y, xMin = vertices[0].x, xMax = vertices[0].x, step, xstep;

	bool first = true;
	bool odd = true;

	
	// find ymax and ymin
	for(auto it = vertices.begin(); it!=vertices.end(); it++){

		yMin = (it->y < yMin) ? it->y : yMin; yMax = (it->y > yMax) ? it->y : yMax;
		xMin = (it->x < xMin) ? it->x : xMin; xMax = (it->x > xMax) ? it->x : xMax;
	}

	step =(yMax - yMin)/600;
	xstep = (xMax - xMin)/800;

	for (float i=yMin; i<=yMax; i+=step){

		glm::vec3 vertex1, vertex2;
		glm::vec3 intersectionPoint;
			
		float dp1, dp2;

		odd = true;
		first = true;

		intersections.clear();
		
		for(auto it = vertices.begin();it != vertices.end(); it++){
	
			// store start vertex of an edge
			if(first){

				vertex1 = *it;
				dp1 = vertex1.y - i;
				first = false;
			}
						
			// store the end vertex of an edge
			else{

				first = true;
				vertex2 = *it;
				dp2 = vertex2.y - i;
			
				// start and end are on opposite sides of the scan line
				if( dp1*dp2 < 0){
					
					intersectionPoint = vertex1 + (vertex2 - vertex1)*(dp1/(dp1-dp2));
					intersections.push_back(intersectionPoint);
				}

				// scan line passes through start point
				else if (dp1 == 0 && dp2 != 0){
					
					vector<glm::vec3> endPoints;

					int counter = 0;

					for (auto iter = vertices.begin(); iter!=vertices.end(); iter++, counter++){
						
						if(iter->x == vertex1.x && iter->y == vertex1.y) {
						
							if(counter%2==0)	
								endPoints.push_back(*(iter+1));
							else
								endPoints.push_back(*(iter-1));
						
					 	}
					}
					
					if(endPoints.size() == 2){ // 2 point case
					
						// if vertex has not already been added
						if(! (find(intersections.begin(),intersections.end(),vertex1)!=intersections.end()) ){
					
							if( (endPoints[0].y - vertex1.y) * (endPoints[1].y - vertex1.y ) < 0 )
								intersections.push_back(vertex1);
						}
					} 			
						
					else if (endPoints.size()>2) {
							
						cout<<"\nJunction no "<<endPointCount<<" detected, has "<<endPoints.size()<<" Convergence";
					
						endPointCount++;
					}
					
				}
				
				// scan line passes through end point
				else if (dp2 == 0 && dp1 != 0){

					vector<glm::vec3> endPoints;

					int counter = 0;

					for (auto iter = vertices.begin(); iter!=vertices.end(); iter++, counter++){
						
						if(iter->x == vertex2.x && iter->y == vertex2.y){
						
							if(counter%2==0)								
								endPoints.push_back(*(iter+1));
							else
								endPoints.push_back(*(iter-1));
						}
					}
						
					if(endPoints.size() == 2){ // 2 point case
						
						// if vertex has not already been added	
						if( !(find(intersections.begin(),intersections.end(),vertex2)!=intersections.end()) ){
						
							if( (endPoints[0].y - vertex2.y) * (endPoints[1].y - vertex2.y ) < 0 )
								intersections.push_back(vertex2);
						}
					} 
	
					else if (endPoints.size()>2) {
							
						cout<<"\nJunction no "<<endPointCount<<" detected, has "<<endPoints.size()<<" Convergence";
					
						endPointCount++;
					}
				}
			}

		}

		// sort intersection points  and push them into addedVertices
                sort(intersections.begin(), intersections.end(),xCoordinateComparision);

		// if the number of intersections is odd, the remove the last intersection
		if(intersections.size() % 2 == 1)
			intersections.erase(intersections.end() - 1);		

		for(auto x = intersections.begin(); x!= intersections.end(); x++){
			addedVertices.push_back(*x);
                }
	}

	// push all added vertices into vertices
	for(auto it = addedVertices.begin(); it!=addedVertices.end(); it++){

		vertices.push_back(*it);
	}
	
	return vertices;

}


int showSlice(slice *s,  GLfloat &x_scale, GLfloat &y_scale, GLfloat &z_scale) {

	vector<glm::vec3> vertices, addedVertices ;

	boundaryVertexCount = 0;
	vertexCount = 0;	
	glm::vec3 temp_vertex;
	
	for ( auto it = s->slice.begin(); it != s->slice.end(); it++ ) {
	
		vertices.push_back(it->startpoint);
		vertices.push_back(it->endpoint);
		
		boundaryVertexCount+=2;
	}

//	fclose(file);

//	vertices = lineFill(vertices);

	GLfloat aspectratio = width/(float)height;
	
	yscale = minimum(xscale,yscale,zscale);
	xscale = yscale;
	zscale = yscale;
	yscale*= aspectratio;	

	x_scale=xscale;
	y_scale=yscale;
	z_scale=zscale;

	glm::mat4 glm_tm = glm::scale(glm::mat4(1.0),glm::vec3(xscale,yscale,zscale));

	GLint tm = glGetUniformLocation(shaderProgram, "transform");
	if(tm ==-1 ) {
		cout<<"\nError, couldn't bind transform ";
			return 1; 
	}



	vertexCount = 0;
	for ( auto it = vertices.begin(); it != vertices.end(); it++) 
			vertexCount++;
	
	glUniformMatrix4fv(tm, 1, GL_FALSE, glm::value_ptr(glm_tm));

	glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*vertices.size(), &vertices[0].x, GL_DYNAMIC_DRAW);

	return 0;
}

int showWindow(slice *s, GLFWwindow* window,GLfloat x_scale, GLfloat y_scale, GLfloat z_scale) {
	 //vertex shader
	while(!glfwWindowShouldClose(window) && cur_slice_no < max_slice_no) {

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//	glDrawArrays(GL_LINES, 0, vertexCount);


	GLint loc = glGetUniformLocation(shaderProgram,"lineColor");


	// fill color
	if(loc!=-1){
	
		glUniform4f(loc,0.0f,0.0f,1.0f,1.0f);
	}
	
	// draw fill
	glDrawArrays(GL_LINES, boundaryVertexCount, vertexCount-boundaryVertexCount+1);	


	// boundary color
	if(loc!=-1){

		glUniform4f(loc,1.0f,0.0f,0.0f,1.0f);
	}
	
	// draw boundary
	glDrawArrays(GL_LINES, 0, boundaryVertexCount);
	
	showSlice(s,x_scale, y_scale, z_scale);	

	glReadPixels (0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixels);

	png::image< png::rgb_pixel > image(800,600);

	int color=0;

	for (size_t y = image.get_height()-1; y >0; --y )
		for (size_t x=0; x < image.get_width(); ++x) {
			image[y][x] = png::rgb_pixel(*(pixels + color++),*(pixels + color++),*(pixels + color++));
		}


	string pngFileName="png/slice_"+(to_string(cur_slice_no))+".png";
	image.write(pngFileName);

	cur_slice_no++; s++;
	
	glfwPollEvents();
	glfwSwapBuffers(window);
	
	}
	return 0;
}
