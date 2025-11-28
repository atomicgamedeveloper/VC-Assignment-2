#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {
	if (!glfwInit()) { // Check that glfw works
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window
	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World!", NULL, NULL);

	if (!window) {
		std::cout << "Failed to create the window!" << std::endl;
		glfwTerminate(); // Free resources
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load opengl function pointers!" << std::endl;
		glfwTerminate();
		return -1;
	}

	const char* vertexShaderSrc =
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n" // Specifying we have an input of vec3 called aPos
		// found at location 0
		"void main() {\n"
		"    gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0f);\n" // Input pos passed to gl_Position
		"}\0"; //GLSL language, like OpenGL

	const char* fragmentShaderSrc =
		"#version 330 core\n"
		"out vec4 fragColor;\n" // Output, all needed to draw a single pixel
		"void main() {\n"
		"    fragColor = vec4(0.0f,0.0f,1.0f,1.0f);\n" //RGBA output
		"}\0";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create empty vertex shader
	// Attach source to shader
	glShaderSource(vertexShader, 1, &vertexShaderSrc, 0); //id, 1 shader, source code pointer, 0
	glCompileShader(vertexShader);

	// Check success of compiling shader
	int success;
	char infoLog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, 0, infoLog); // Try to get info log if it was not successful
		std::cout << "Failed to compile vertex shader! ERROR: " << infoLog << std::endl;
	}

	// Repeat for Fragment Shader

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSrc, 0);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, 0, infoLog);
		std::cout << "Failed to compile fragment shader! ERROR: " << infoLog << std::endl;
	}

	// Create shader program with both shaders tied to it
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram,vertexShader);
	glAttachShader(shaderProgram,fragmentShader);
	// Combine
	glLinkProgram(shaderProgram);
	// Check success
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, 0, infoLog);
		std::cout << "Failed to link shader program! ERROR: " << infoLog << std::endl;
	}

	// Inside shader program, free up resources
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Draw triangle (normalized coordinates)
	float vertices[] = {
		-0.5f,-0.5f,0.0f,
		0.0f,0.5f,0.0f,
		0.5f,-0.5f,0.0f,
	};

	// Input to graphics pipeline - vertex shader
	// Creating memory on GPU
	unsigned int VAO, VBO; // create vertex buffer object and vertex array object (saving specifications instead of reinitializing again)
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind it and specify type (array buffer)

	// Fill memory with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,GL_STATIC_DRAW); // static = set only once, drawn a lot
	// gl_stream_draw = set once, used by gpu just a few times 
	// gl_dynamic_draw = data is changed a lot and drawn a lot

	// How to interpret memory
	// From vertex shader, location 0, size is 3 as vec3, float (opengl type),
	// no normalization, could be 0 and let open gl determine it or write it as per coordinate size,
	// start of the veretx - casted 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0); // Enable vertex attrib pointer.

	// Unbind everything.
	// Can be binded whenever needed through vertex array
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0.6f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); // Essential before rendering a new scene, blank and ready!

		glUseProgram(shaderProgram); // Use shader program
		glBindVertexArray(VAO); // Bind vertex array
		glDrawArrays(GL_TRIANGLES, 0, 3); // kind, start vertex, end vertex

		glfwSwapBuffers(window); // Work with other buffer
		glfwPollEvents(); // Process pending inputs
	}

	// Free resources
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	std::cout << "window size is " << width << " x " << height << std::endl;
	glViewport(0, 0, width, height); // Resetting the rendering area (view port)
}
