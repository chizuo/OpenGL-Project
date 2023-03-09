#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// additional code to create the coloring cycle
float colors[][3] = {
	{0.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f}};

float Time = 0.0f;

// settings changed cause my laptop is 1920x1080 and was causing flickering
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// ========================================
// GLSL (OpenGL Shading Language)
// Modern OpenGL requires that we at least set up a vertex and fragment
// shader if we want to do some rendering
const char *vertexShaderSource =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec3 ourColor;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(aPos, 1.0);\n"
	"   ourColor = aColor;\n"
	"}\0";

// Fragment shader
// The fragment shader is all about calculating the color output of your pixels.
const char *fragmentShaderSource =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec3 ourColor;\n"
	"void main()\n"
	"{\n"
	"   FragColor = vec4(ourColor, 1.0f);\n"
	"}\n\0";
// ========================================

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "COMP 565 - Advanced Computer Graphics - OpenGL Basics", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	// link shaders
	// A shader program object is the final linked version of multiple shaders combined.
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions         // colors
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right -- RED
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left -- GREEN
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f	  // top -- BLUE
	};

	// Vertex Buffer Objects and Vertex Array Object
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	// 1) The first parameter specifies which vertex attribute we want to configure.
	// Remember that we specified the location of the position vertex attribute in the vertex shader with layout (location = 0).
	// This sets the location of the vertex attribute to 0 and since we want to pass data to this vertex attribute, we pass in 0.
	// 2) The next argument specifies the size of the vertex attribute. The vertex attribute is a vec3 so it is composed of 3 values.
	// 3) The third argument specifies the type of the data which is GL_FLOAT (a vec* in GLSL consists of floating point values).
	// 4) The next argument specifies if we want the data to be normalized. If we're inputting integer data types (int, byte)
	// and we've set this to GL_TRUE, the integer data is normalized to 0 (or -1 for signed data) and 1 when converted to float.
	// This is not relevant for us so we'll leave this at GL_FALSE.
	// 5) The fifth argument is known as the stride and tells us the space between consecutive vertex attribute sets. Since the next
	// set of position data is located exactly 3 Times the size of a float away we specify that value as the stride. Note that since
	// we know that the array is tightly packed (there is no space between the next vertex attribute value) we could've also specified
	// the stride as 0 to let OpenGL determine the stride (this only works when values are tightly packed).
	// 6) The last parameter is of type void* and thus requires that weird cast. This is the offset of where the position data begins
	// in the buffer. Since the position data is at the start of the data array this value is just 0.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// as we only have a single shader, we could also just activate our shader once beforehand if we want to
	glUseProgram(shaderProgram);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		for (int i = 0; i < 3; i++)
		{
			/*
				using sinf or cosf to get floating point values between 0 to 1
				for the purpose of creating the fade in and fade out
			*/
			float fader = (cosf(Time + i * 1.0f) + 1.0f) / 2.0f;

			/*
				changes bottom right color at i = 0 => vertices[3] = colors[0][0] = 0.0f, vertices[4] = colors[0][1] = 0.0f, vertices[5] = colors[0][2] = 1.0f
				changes bottom left  color at i = 1 => vertices[9] = colors[1][0] = 1.0f, vertices[10] = colors[1][1] = 0.0f, vertices[11] = colors[1][2] = 0.0f
				changes top          color at i = 2 => vertices[15] = colors[2][0] = 1.0f, vertices[16] = colors[2][1] = 1.0f, vertices[17] = colors[2][2] = 0.0f
			*/
			vertices[i * 6 + 3] = colors[i][0] * fader;
			vertices[i * 6 + 4] = colors[i][1] * fader;
			vertices[i * 6 + 5] = colors[i][2] * fader;
		}

		// Update the vertex buffer object with the updated values of the vertex attributes
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// render the triangle
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		Time += 0.01f;

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
