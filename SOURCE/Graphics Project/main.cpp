#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>
#include <glm\trigonometric.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "objloader.hpp"
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>

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
GLuint loadBMP(const char * imagepath);


int main(int argc, const char * argv[]){

	rapidjson::Document jsonParser;

	FILE *fp;
	fopen_s(&fp, "InputFile.txt", "rb+");
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = (char*)malloc(len + 1);
	fread(buffer, len, 1, fp);
	fclose(fp);
	buffer[len] = 0;
		jsonParser.Parse(buffer);
	free(buffer);

	int width = jsonParser["SIZE"]["width"].GetInt();
	int height = jsonParser["SIZE"]["height"].GetInt();

	int lookFromX = jsonParser["CAMERA"]["lookFromX"].GetInt();
	int lookFromY = jsonParser["CAMERA"]["lookFromY"].GetInt();
	int lookFromZ = jsonParser["CAMERA"]["lookFromZ"].GetInt();
	int lookAtX = jsonParser["CAMERA"]["lookAtX"].GetInt();
	int lookAtY = jsonParser["CAMERA"]["lookAtY"].GetInt();
	int lookAtZ = jsonParser["CAMERA"]["lookAtZ"].GetInt();
	int upX = jsonParser["CAMERA"]["upX"].GetInt();
	int upY = jsonParser["CAMERA"]["upY"].GetInt();
	int upZ = jsonParser["CAMERA"]["upZ"].GetInt();
	float fov = (float)(jsonParser["CAMERA"]["fov"].GetDouble());

	float lightX = (float)(jsonParser["LIGHT"]["x"].GetDouble());
	float lightY = (float)(jsonParser["LIGHT"]["y"].GetDouble());
	float lightZ = (float)(jsonParser["LIGHT"]["z"].GetDouble());
	float lightW = (float)(jsonParser["LIGHT"]["w"].GetDouble());
	float lightR = (float)(jsonParser["LIGHT"]["r"].GetDouble());
	float lightG = (float)(jsonParser["LIGHT"]["g"].GetDouble());
	float lightB = (float)(jsonParser["LIGHT"]["b"].GetDouble());
	float lightA = (float)(jsonParser["LIGHT"]["a"].GetDouble());

	float ambientR = (float)(jsonParser["AMBIENT"]["r"].GetDouble());
	float ambientG = (float)(jsonParser["AMBIENT"]["g"].GetDouble());
	float ambientB = (float)(jsonParser["AMBIENT"]["b"].GetDouble());
	float ambientA = (float)(jsonParser["AMBIENT"]["a"].GetDouble());

	float diffuseR = (float)(jsonParser["DIFFUSE"]["r"].GetDouble());
	float diffuseG = (float)(jsonParser["DIFFUSE"]["g"].GetDouble());
	float diffuseB = (float)(jsonParser["DIFFUSE"]["b"].GetDouble());
	float diffuseA = (float)(jsonParser["DIFFUSE"]["a"].GetDouble());

	float specularR = (float)(jsonParser["SPECULAR"]["r"].GetDouble());
	float specularG = (float)(jsonParser["SPECULAR"]["g"].GetDouble());
	float specularB = (float)(jsonParser["SPECULAR"]["b"].GetDouble());
	float specularA = (float)(jsonParser["SPECULAR"]["a"].GetDouble());

	float emissionR = (float)(jsonParser["EMISSION"]["r"].GetDouble());
	float emissionG = (float)(jsonParser["EMISSION"]["g"].GetDouble());
	float emissionB = (float)(jsonParser["EMISSION"]["b"].GetDouble());
	float emissionA = (float)(jsonParser["EMISSION"]["a"].GetDouble());

	float shininess = (float)(jsonParser["SHININESS"]["s"].GetDouble());

	if (!glfwInit()){
		std::cout << "GLFW Failed to Initalize" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	GLFWwindow *tmp = glfwCreateWindow(480, 240, "", NULL, NULL);
	glfwMakeContextCurrent(tmp);

	glewInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	GLFWwindow *mainWindow = glfwCreateWindow(width, height, "GLFW Window", NULL, NULL);
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE); // Cull triangles which normal is not towards the camera
	glEnable(GL_DEPTH_TEST); // Enable depth test
	glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one


	bool error;
	GLuint projectProgramID = compileShader("Data\\ModelVertexShader.txt", "Data\\ModelFragmentShader.txt", error);

	GLuint MVP_UniformID = glGetUniformLocation(projectProgramID, "MVP");
	GLuint M_UniformID = glGetUniformLocation(projectProgramID, "M");
	GLuint V_UniformID = glGetUniformLocation(projectProgramID, "V");
	GLuint Texture_UniformID = glGetUniformLocation(projectProgramID, "myTextureSampler");
	GLuint Light_Pos_UniformID = glGetUniformLocation(projectProgramID, "lightPosition");
	GLuint Light_Color_UniformID = glGetUniformLocation(projectProgramID, "lightCol");
		glm::vec4 lightColor = glm::vec4(lightR, lightG, lightB, lightA);
	GLuint Ambient_UniformID = glGetUniformLocation(projectProgramID, "ambient");
		glm::vec4 ambient = glm::vec4(ambientR, ambientG, ambientB, ambientA);
	GLuint Diffuse_UniformID = glGetUniformLocation(projectProgramID, "diffuse");
		glm::vec4 diffuse = glm::vec4(diffuseR, diffuseG, ambientB, ambientA);
	GLuint Specular_UniformID = glGetUniformLocation(projectProgramID, "specular");
		glm::vec4 specular = glm::vec4(specularR, specularG, specularB, specularA);
	GLuint Emission_UniformID = glGetUniformLocation(projectProgramID, "emission");
		glm::vec4 emission = glm::vec4(emissionR, emissionG, emissionB, emissionA);
	GLuint Shininess_UniformID = glGetUniformLocation(projectProgramID, "s");

	// Read our model file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	bool res = loadOBJ("Data\\suzanne.model", vertices, uvs, normals);
	
	GLuint trashTexture = loadBMP("Data\\grass.bmp");

	GLuint modelBuffer;
	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint modelUVBuffer;
	glGenBuffers(1, &modelUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	//VAO
	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //Attribute, Size of Input, Type, normalized, stride, offset

	glBindBuffer(GL_ARRAY_BUFFER, modelUVBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0); //Disables writes to our VAO

	glm::mat4 perspective = glm::perspective(glm::radians(fov), (float(width) / float(height)), 0.1f, 100.0f);
	glm::mat4 ortho = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	glm::mat4 Projection = perspective;
	bool projectionToggle = true;

	glm::mat4 projectModel = glm::mat4(1.0f);
	projectModel = glm::translate(projectModel, glm::vec3(0.0f, 0.0f, 0.0f));

	glm::mat4 initView = glm::lookAt(
		glm::vec3(lookFromX, lookFromY, lookFromZ),
		glm::vec3(lookAtX, lookAtY, lookAtZ),
		glm::vec3(upX, upY, upZ)
	);

	glm::mat4 View = initView;

	glClearColor(120.0f / 255.0f, 96.0f / 255.0f, 96.0f / 255.0f, 1.0f);

	while (!glfwWindowShouldClose(mainWindow)){

		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		else if (glfwGetKey(mainWindow, GLFW_KEY_R) == GLFW_PRESS) {
			View = initView;
		}
		else if (glfwGetKey(mainWindow, GLFW_KEY_C) == GLFW_PRESS){
			if (projectionToggle)
				Projection = ortho;
			else Projection = perspective;
			projectionToggle = !projectionToggle;
		}

		glm::mat4 modelMVP = Projection * View * projectModel;

		//MODEL
		glUseProgram(projectProgramID);

		glUniformMatrix4fv(MVP_UniformID, 1, GL_FALSE, &modelMVP[0][0]);
		glUniformMatrix4fv(M_UniformID, 1, GL_FALSE, &projectModel[0][0]);
		glUniformMatrix4fv(V_UniformID, 1, GL_FALSE, &View[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, trashTexture);
		glUniform1i(Texture_UniformID, 0);

		glUniform3f(Light_Pos_UniformID, lightX, lightY, lightZ);
		glUniform4fv(Light_Color_UniformID, 1, lightColor.data);
		glUniform4fv(Ambient_UniformID, 1, ambient.data);
		glUniform4fv(Diffuse_UniformID, 1, diffuse.data);
		glUniform4fv(Specular_UniformID, 1, specular.data);
		glUniform4fv(Emission_UniformID, 1, emission.data);
		glUniform1f(Shininess_UniformID, shininess);

		glBindVertexArray(vertexArrayID);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		//END MODEL

		glfwSwapBuffers(mainWindow);
	}

	glfwTerminate();
    return 0;
}


GLuint loadBMP(const char * imagepath) {

	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
	unsigned char * data; // Actual RGB data

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("Image could not be opened\n"); return 0; }

	if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return 0;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}

	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

										 
	data = new unsigned char[imageSize]; // Create a buffer
	fread(data, 1, imageSize, file); // Read the actual data from the file into the buffer
	fclose(file); //Everything is in memory now, the file can be closed

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID); // "Bind" the newly created texture : all future texture functions will modify this texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data); // Give the image to OpenGL
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return textureID;
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