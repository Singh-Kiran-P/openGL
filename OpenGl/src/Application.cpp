//Wacht https://www.youtube.com/watch?v=DE6Xlx_kbo0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=11&ab_channel=TheCherno

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace std;

/* Macro for creating automatic a breakpoint on a line that breaks the code [Error handling] */
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

/* Error handling */
static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

/* Check if there was an error */
// Rap all openGl func inside this function
static bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		cout << "[OpenGL Error] (" << error << ")" << function <<
			" " << file << ":" << line << endl;
		return false;
	}

	return true;
}


struct ShaderProgramSource
{
	string VertexSource;
	string FragmentSource;
};


static ShaderProgramSource ParseShader(const string& filepath)
{
	ifstream shaderIn(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	string line;
	stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(shaderIn, line))
	{
		if (line.find("#shader") != string::npos)
		{
			if (line.find("vertex") != string::npos)
				//set mode to vertex
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != string::npos)
				//set mode to fragment
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static unsigned int complileShader(unsigned int type, const string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	int res;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &res));
	if (res == GL_FALSE)
	{
		int len;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len));
		char* message = (char*)alloca(len * sizeof(char));
		GLCall(glGetShaderInfoLog(id, len, &len, message));
		cout << "Failt to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << endl;
		cout << message << endl;
		GLCall(glDeleteShader(id));
		return 0;
	}

	return id;
}

static unsigned int createShader(const string& vertexShader, const string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = complileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = complileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;

}


int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* change the frame rate */
	glfwSwapInterval(1);

	/* init Glew Linker */
	GLenum err = glewInit();
	if (GLEW_OK != err)
		cout << "Error while initailising Glew";

	cout << glGetString(GL_VERSION) << endl;

	/* Triangle positions */
	float positions[] =
	{
		-0.5f, -0.5f, // 0
		 0.5f, -0.5f, // 1
		 0.5f,  0.5f, // 2
		-0.5f,  0.5f, // 3

	};

	unsigned int indices[] =
	{
		0,1,2,
		2,3,0
	};


	/* Define triangle vertex render buffer */
	unsigned int buffer;
	GLCall(glGenBuffers(1, &buffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

	/* After the buffer is bind, create Vertex Attribute */
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

	/* Define vertex index buffer */
	unsigned int indexBufferId;
	GLCall(glGenBuffers(1, &indexBufferId));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));


	/* Read shaders from file and create shaders */
	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
	cout << "VERTEX" << endl;
	cout << source.VertexSource << endl;
	cout << "FRAGMENT" << endl;
	cout << source.FragmentSource << endl;

	unsigned int shader = createShader(source.VertexSource, source.FragmentSource);
	GLCall(glUseProgram(shader));

	/* Uniform variable from the basic.shader file
		- to set data into the shader file (vec4 -> 4f)
	*/
	GLCall(int location = glGetUniformLocation(shader, "u_Color"));
	ASSERT(location != -1);
	GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));

	float r = 0.0f;
	float increment = 0.05f;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Call to make a the triangle with index buffer( must be unsigned int) */
		GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		/* animate box color */
		if (r > 1.0f)
			increment = -0.05f;
		else if (r < 0.0f)
			increment = 0.05f;
		r += increment;
		


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	//glDeleteProgram(shader);

	glfwTerminate();
	return 0;
}