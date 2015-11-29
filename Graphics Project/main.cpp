#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>
#include <glm\trigonometric.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

GLuint compileShader(std::string vertexPath, std::string fragPath, bool &error);
bool loadOBJ(
	     const char * path,
	     std::vector < glm::vec3 > & out_vertices,
	     std::vector < glm::vec2 > & out_uvs,
	     std::vector < glm::vec3 > & out_normals
	);

//draw counter clockwise 
//double sided triangle!
static const GLfloat gTriangle[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
	0.0f, 1.0f, 0.0f
};

static const GLfloat gTriangleColors[] = {
	0.583f,  0.771f,  0.014f,
	0.609f,  0.115f,  0.436f,
	0.327f,  0.483f,  0.844f,
	0.195f,  0.548f,  0.859f,
	0.483f,  0.896f,  0.789f,
	0.771f,  0.328f,  0.270f
};

static const GLfloat gCube[] = {
	- 1.0f,-1.0f,-1.0f, // triangle 1 : begin
	- 1.0f,-1.0f, 1.0f,
	- 1.0f, 1.0f, 1.0f, // triangle 1 : end
	1.0f, 1.0f,-1.0f, // triangle 2 : begin
	- 1.0f,-1.0f,-1.0f,
	- 1.0f, 1.0f,-1.0f, // triangle 2 : end
	1.0f,-1.0f, 1.0f,
	- 1.0f,-1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	- 1.0f,-1.0f,-1.0f,
	- 1.0f,-1.0f,-1.0f,
	- 1.0f, 1.0f, 1.0f,
	- 1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	- 1.0f,-1.0f, 1.0f,
	- 1.0f,-1.0f,-1.0f,
	- 1.0f, 1.0f, 1.0f,
	- 1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	- 1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	- 1.0f, 1.0f,-1.0f,
	- 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	- 1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f
};


static const GLfloat gCubeColors[] = {
	    0.583f,  0.771f,  0.014f,
	    0.609f,  0.115f,  0.436f,
	    0.327f,  0.483f,  0.844f,
	    0.822f,  0.569f,  0.201f,
	    0.435f,  0.602f,  0.223f,
	    0.310f,  0.747f,  0.185f,
	    0.597f,  0.770f,  0.761f,
	     0.559f,  0.436f,  0.730f,
	     0.359f,  0.583f,  0.152f,
	     0.483f,  0.596f,  0.789f,
	     0.559f,  0.861f,  0.639f,
	     0.195f,  0.548f,  0.859f,
	     0.014f,  0.184f,  0.576f,
	     0.771f,  0.328f,  0.970f,
	     0.406f,  0.615f,  0.116f,
	     0.676f,  0.977f,  0.133f,
	     0.971f,  0.572f,  0.833f,
	     0.140f,  0.616f,  0.489f,
	     0.997f,  0.513f,  0.064f,
	     0.945f,  0.719f,  0.592f,
	    0.543f,  0.021f,  0.978f,
	     0.279f,  0.317f,  0.505f,
	     0.167f,  0.620f,  0.077f,
	     0.347f,  0.857f,  0.137f,
	0.055f,  0.953f,  0.042f,
	0.714f,  0.505f,  0.345f,
	0.783f,  0.290f,  0.734f,
	0.722f,  0.645f,  0.174f,
	0.302f,  0.455f,  0.848f,
	0.225f,  0.587f,  0.040f,
	0.517f,  0.713f,  0.338f,
	0.053f,  0.959f,  0.120f,
	0.393f,  0.621f,  0.362f,
	0.673f,  0.211f,  0.457f,
	0.820f,  0.883f,  0.371f,
	0.982f,  0.099f,  0.879f
};


int main(int argc, const char * argv[]){
	if (!glfwInit()){
		std::cout << "GLFW Failed to Initalize" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	GLFWwindow *tmp = glfwCreateWindow(480, 240, "", NULL, NULL);
	glfwMakeContextCurrent(tmp);

	glewInit();

	// Double check the version
	GLint glVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

	double v = (double)glVersion[0];
	v += (double)glVersion[1] / 10.0;
	printf("OpenGL Version:%s\n", glGetString(GL_VERSION));
	printf("OpenGL %f\n", v);
	glfwDestroyWindow(tmp);
	tmp = nullptr;

	if (v < 3.3){
		printf("Your graphics card does not support OpenGL 3.3 or Newer!\n");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	GLFWwindow *mainWindow = glfwCreateWindow(800, 600, "GLFW Window", NULL, NULL);
	if (!mainWindow){
		std::cout << "GLFW Failed to Create a Window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(mainWindow);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (GLEW_OK != err){
		fprintf(stderr, "GLEW ERROR: %s\n", glewGetErrorString(err));
		return 1;
	}
	
	glfwSwapInterval(1); //Locks draws to monitors refresh rate (60fps)
	glEnable(GL_CULL_FACE); // Cull triangles which normal is not towards the camera
	glEnable(GL_DEPTH_TEST); // Enable depth test
	glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one

	bool error;
	GLuint programID = compileShader("SimpleVertexShader.txt", "SimpleFragmentShader.fsh", error);
	glUseProgram(programID);

	glClearColor(120.0f / 255.0f, 96.0f / 255.0f, 96.0f / 255.0f, 1.0f);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	GLuint vertexbufferTriangle;
	glGenBuffers(1, &vertexbufferTriangle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferTriangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gTriangle), gTriangle, GL_STATIC_DRAW);

	GLuint colorbufferTriangle;
	glGenBuffers(1, &colorbufferTriangle);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferTriangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof( gTriangleColors ), gTriangleColors, GL_STATIC_DRAW);

	GLuint vertexbufferCube;
	glGenBuffers(1, &vertexbufferCube);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gCube), gCube, GL_STATIC_DRAW);

	GLuint colorbufferCube;
	glGenBuffers(1, &colorbufferCube);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferCube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gCubeColors), gCubeColors, GL_STATIC_DRAW);

	// Read our .obj file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals; // Won't be used at the moment.
	bool res = loadOBJ("Banana.model", vertices, uvs, normals);
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	glm::mat4 triangleModel = glm::mat4(1.0f);
	triangleModel = glm::translate(triangleModel, glm::vec3(3.0f, 0.0f, 0.0f));

	glm::mat4 cubeModel = glm::mat4(1.0f);
	cubeModel = glm::translate(cubeModel, glm::vec3(-3.0f, 0.0f, 0.0f));	

	glm::mat4 initView = glm::lookAt(
		glm::vec3(0, 1, -10),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
	);

	glm::mat4 View = initView;

	while (!glfwWindowShouldClose(mainWindow)){

		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLuint MatrixID = glGetUniformLocation(programID, "MVP");

		if (glfwGetKey(mainWindow, GLFW_KEY_UP) == GLFW_PRESS) {
			View = glm::scale(View, glm::vec3(1.01, 1.01f, 1.01f));
		}
		else if (glfwGetKey(mainWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
			View = glm::scale(View, glm::vec3(0.99f, 0.99f, 0.99f));
		}
		else if (glfwGetKey(mainWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
			View = glm::rotate(View, .01f, glm::vec3(0,1,0));
		}
		else if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			View = glm::rotate(View, -.01f, glm::vec3(0, 1, 0));
		}
		else if (glfwGetKey(mainWindow, GLFW_KEY_R)) {
			View = initView;
		}

		glm::mat4 triangleMVP = Projection * View * triangleModel;
		glm::mat4 cubeMVP = Projection * View * cubeModel;

		//TRIANGLE
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &triangleMVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferTriangle);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbufferTriangle);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		//END TRIANGLE


		//CUBE
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &cubeMVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbufferCube);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the cube !
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		glDisableVertexAttribArray(0);
		//END CUBE


		glfwSwapBuffers(mainWindow);
	}

	glfwTerminate();
    return 0;
}


bool loadOBJ(
	const char * path,
	std::vector < glm::vec3 > & out_vertices,
	std::vector < glm::vec2 > & out_uvs,
	std::vector < glm::vec3 > & out_normals
	) {

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {
		char lineHeader[256];
		int res = fscanf(file, "%s", lineHeader); // read the first word of the line
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}
	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		out_vertices.push_back(vertex);
	}

	return true;
}




GLuint compileShader(std::string vertexPath, std::string fragPath, bool &error)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertexPath, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragPath, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertexPath.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);

	void *errorExists;

	if (VertexShaderErrorMessage.size() != 0)
	{
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);

		errorExists = &VertexShaderErrorMessage[0];
		if (errorExists != nullptr)
		{
			//error = true;
			fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
		}
		errorExists = nullptr;
	}
		

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragPath.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);

	if (FragmentShaderErrorMessage.size() != 0)
	{
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		errorExists = &FragmentShaderErrorMessage[0];
		if (errorExists != nullptr)
		{
			//error = true;
			fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
		}
	}

	errorExists = nullptr;

	// Link the program
	fprintf(stdout, "Linking shader program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage(std::max(InfoLogLength, int(1)));
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}