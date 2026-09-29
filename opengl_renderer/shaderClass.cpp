#include"shaderClass.h"

// Reads a text file and outputs a string with everything in the text file
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);		//vertex shaderObj and its Ref 
	glShaderSource(vertexShader, 1, &vertexSource, NULL);		//connect the vertex souce code to shaderObj
	glCompileShader(vertexShader);								//vertex Shader to Machine Code
	compileErrors(vertexShader, "VERTEX");


	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);		//fragment shaderObj and its Ref
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);		//connect fragment source code to shaderObj
	glCompileShader(fragmentShader);								//fragment shader to machine code
	compileErrors(fragmentShader, "FRAGMENT");

	ID = glCreateProgram();					//create shader program and its Ref

	glAttachShader(ID, vertexShader);		//attach the vertex and fragment
	glAttachShader(ID, fragmentShader);		//shader to shaderProgram
	glLinkProgram(ID);						//Link-up all the shaders to the shaderProgram
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);			//delete useless shader
	glDeleteShader(fragmentShader);			//delete useless shader

}

void Shader::Activate()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::compileErrors(unsigned int shader, const char* type)
{
	GLint hasCompiled;
	char infoLog[1024];

	if (std::string(type) != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for: " << type << "\n"
				<< infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for: " << type << "\n"
				<< infoLog << std::endl;
		}
	}
}
