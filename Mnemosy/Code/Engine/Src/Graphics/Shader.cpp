#include "Engine/Include/Graphics/Shader.h"

#include "Engine/Include/Core/Log.h"
#include "Engine/Include/Graphics/Utils/ShaderIncludeParser.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>


namespace mnemosy::graphics
{
	// public

	Shader::Shader(const char* vertexPath, const char* fragmentPath)
	{
		CreateShaderProgram(vertexPath, fragmentPath);
	};

	Shader::~Shader()
	{
		DeleteShaderProgram();
	}

	void Shader::CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
	{
		if (ID)
		{
			MNEMOSY_WARN("A Shader program already exists under this ID: {} \nFilepath Vertex Shader: {}\nFilepath Fragment Shader: {} \n -> Proceding to override shader program", ID, m_pathVertex, m_pathFragment);
			DeleteShaderProgram();
		}
		m_pathVertex = vertexPath;
		m_pathFragment = fragmentPath;

		std::string vertexCode = Shadinclude::load(vertexPath, "#include");
		std::string fragmentCode = Shadinclude::load(fragmentPath, "#include");

		const char* vertexShaderCode = vertexCode.c_str();
		const char* fragmentShaderCode = fragmentCode.c_str();


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
		CheckCompileErrors(vertexShader, "VERTEX");
		// compile fragment
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
		glCompileShader(fragmentShader);
		CheckCompileErrors(fragmentShader, "FRAGMENT");


		ID = glCreateProgram();
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glLinkProgram(ID);
		CheckCompileErrors(ID, "PROGRAM");

		MNEMOSY_DEBUG("Compiled Shader Program:\n	VertexShader: {}\n	FragmentShader: {}", vertexPath, fragmentPath);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void Shader::Use()
	{
		glUseProgram(ID);
	}


	void Shader::SetUniformBool(const std::string& name, bool value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform1i(uniformLocation, (int)value);
	}

	void Shader::SetUniformInt(const std::string& name, int value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform1i(uniformLocation, value);
	}

	void Shader::SetUniformFloat(const std::string& name, float value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform1f(uniformLocation, value);
	}
	void Shader::SetUniformFloat2(const std::string& name, float x, float y) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform2f(uniformLocation, x, y);
	}
	void Shader::SetUniformFloat3(const std::string& name, float x, float y, float z) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform3f(uniformLocation, x, y, z);
	}
	void Shader::SetUniformFloat4(const std::string& name, float x, float y, float z, float w) const
	{
		int uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform4f(uniformLocation, x, y, z, w);
	}
	void Shader::SetUniformMatrix4(const std::string& name, glm::mat4 matrix) {
		unsigned int uniformLocation = glGetUniformLocation(ID, name.c_str());

		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void Shader::DeleteShaderProgram()
	{
		if (ID)
		{
			glDeleteProgram(ID);
			ID = NULL;
		}
	}
	// private

	void Shader::CheckCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				MNEMOSY_ERROR("SHADER_COMPILATION_ERROR of type: {} \nInfo Log: {}\n Filepath Vertex Shader: {}\n Filepath Fragment Shader: {}", type, infoLog, m_pathVertex, m_pathFragment);
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				MNEMOSY_ERROR("SHADER_PROGRAM_LINKING_ERROR of type: {} \nInfo Log: {}\n Filepath Vertex Shader: {}\n Filepath Fragment Shader: {}", type, infoLog, m_pathVertex, m_pathFragment);
			}
		}
	}

} // end namespace mnemosy::graphics