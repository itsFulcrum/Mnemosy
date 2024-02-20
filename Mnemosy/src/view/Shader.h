#ifndef SHADER_H
#define SHADER_H

//#include <glad/glad.h>

#include <string>
//#include <fstream>
//#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// because glsl doesn't support #include I use this libary to parse the source files and add includes
// it supports nested includes but NOT headerguards like #ifdef
// IT ALSO DOES NOT INGORE COMMENTS!
#include <Shadinclude.hpp>

class Shader {
public:
	unsigned int ID;
	std::string pathVertex;
	std::string pathFragment;

	Shader()
	{
	};
	Shader(const char* vertexPath, const char* fragmentPath)
	{
		CreateShaderProgram(vertexPath, fragmentPath);
	};

	~Shader()
	{
		glDeleteProgram(ID);
	}

	void CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
	{
		std::string vertexCode;
		std::string fragmentCode;

		/*
		std::ifstream vertexShaderFile;
		std::ifstream fragmentShaderFile;
		try
		{



		}
		catch (std::ifstream::failure error)
		{
			std::cout << "ERROR::SHADER:: could not read shader code from file" << std::endl;
		}
		*/
		

		vertexCode = Shadinclude::load(vertexPath, "#include");
		fragmentCode = Shadinclude::load(fragmentPath, "#include");


		const char* vertexShaderCode = vertexCode.c_str();
		const char* fragmentShaderCode = fragmentCode.c_str();

		pathVertex = vertexPath;
		pathFragment = fragmentPath;

		// compile shaders
		//int success;
		//char infoLog[512];

		unsigned int vertexShader;
		unsigned int fragmentShader;
		// compile vertex
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
		glCompileShader(vertexShader);
		// print compile Errors upon failure
		checkCompileErrors(vertexShader, "VERTEX");
		// compile fragment
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
		glCompileShader(fragmentShader);
		checkCompileErrors(fragmentShader, "FRAGMENT");


		ID = glCreateProgram();
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");


		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	
	// use / activate the shader
	void use() 
	{
		glUseProgram(ID);
	}
	
	void setUniformBool(const std::string& name, bool value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform1i(uniformLocation, (int)value);
	}
	void setUniformInt(const std::string& name, int value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform1i(uniformLocation, value);
	}
	void setUniformFloat(const std::string& name, float value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform1f(uniformLocation, value);
	}
	void setUniformFloat2(const std::string& name, float x, float y) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform2f(uniformLocation, x, y);
	}
	void setUniformFloat3(const std::string& name, float x, float y, float z) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform3f(uniformLocation, x,y,z);
	}
	void setUniformFloat4(const std::string& name, float x, float y, float z,float w) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform4f(uniformLocation, x, y, z,w);
	}
	void setUniformMatrix4(const std::string& name, glm::mat4 matrix ) {
		unsigned int uniformLocation = glGetUniformLocation(ID, name.c_str());
		
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(matrix));
	}


private:
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
				std::cout << "filePath:" << "\n" << pathVertex<< "\n" << pathFragment << "\n" << "-- -------------------------------------------------- - -- " << std::endl;
				
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
				std::cout << "filePath:" << "\n" << pathVertex << "\n" << pathFragment << "\n" << "-- -------------------------------------------------- - -- " << std::endl;
			}
		}
	
	
	}
};

#endif

