#include "Include/Graphics/Shader.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/Utils/ShaderIncludeParser.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace mnemosy::graphics
{
	// public

	Shader::Shader(const char* vertexPath, const char* fragmentPath) {

		CreateShaderProgram(vertexPath, fragmentPath);
	}

	Shader::Shader(const char* MeshPath, const char* fragmentPath, bool isMeshShader) {

		CreateMeshShaderProgramm(MeshPath,fragmentPath);
	}

	Shader::~Shader() {
		DeleteShaderProgram();
	}

	bool Shader::CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
	{
		if (ID) {
			//MNEMOSY_WARN("Shader::CreateShaderProgram: A Shader program already exists under this ID: {} -> Proceding to override shader program");
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

		bool compilationSuccessfull = CheckCompileErrors(ID, "PROGRAM");

		if (compilationSuccessfull) {
			MNEMOSY_DEBUG("Compiled Shader Program: \n - VertexShader: {} - FragmentShader: {}", vertexPath, fragmentPath);
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return compilationSuccessfull;
	}

	bool Shader::CreateMeshShaderProgramm(const char* meshPath, const char* fragmentPath) {
		if (ID)
		{
			//MNEMOSY_WARN("A Shader program already exists under this ID: {} \nFilepath Vertex Shader: {}\nFilepath Fragment Shader: {} \n -> Proceding to override shader program", ID, m_pathVertex, m_pathFragment);
			DeleteShaderProgram();
		}
		m_pathVertex = meshPath;
		m_pathFragment = fragmentPath;

		std::string vertexCode = Shadinclude::load(meshPath, "#include");
		std::string fragmentCode = Shadinclude::load(fragmentPath, "#include");

		const char* vertexShaderCode = vertexCode.c_str();
		const char* fragmentShaderCode = fragmentCode.c_str();


		// compile shaders
		

		unsigned int vertexShader;
		unsigned int fragmentShader;
		// compile vertex
		vertexShader = glCreateShader(GL_MESH_SHADER_NV);
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


		bool compilationSuccess = true;
		compilationSuccess = CheckCompileErrors(ID, "PROGRAM");

		MNEMOSY_DEBUG("Compiled Shader Program:\nMeshShader: {}\nFragmentShader: {}", meshPath, fragmentPath);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return compilationSuccess;
	}

	void Shader::Use()
	{
		glUseProgram(ID);
	}


	void Shader::SetUniformBool(const char* name, bool value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name);
		glUniform1i(uniformLocation, (int)value);
	}

	void Shader::SetUniformInt(const char* name, int value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name);
		glUniform1i(uniformLocation, value);
	}

	void Shader::SetUniformFloat(const char* name, float value) const
	{
		int uniformLocation = glGetUniformLocation(ID, name);
		glUniform1f(uniformLocation, value);
	}
	void Shader::SetUniformFloat2(const char* name, float x, float y) const
	{
		int uniformLocation = glGetUniformLocation(ID, name);
		glUniform2f(uniformLocation, x, y);
	}
	void Shader::SetUniformFloat3(const char* name, float x, float y, float z) const
	{
		int uniformLocation = glGetUniformLocation(ID, name);
		glUniform3f(uniformLocation, x, y, z);
	}
	void Shader::SetUniformFloat4(const char* name, float x, float y, float z, float w) const
	{
		int uniformLocation = glGetUniformLocation(ID, name);
		glUniform4f(uniformLocation, x, y, z, w);
	}

	void Shader::SetUniformMatrix4(const char* name, const glm::mat4& matrix) {
		unsigned int uniformLocation = glGetUniformLocation(ID, name);

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

	bool Shader::CheckCompileErrors(unsigned int shader,const std::string& type) {


		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				MNEMOSY_ERROR("SHADER_COMPILATION_ERROR of type: {} \nInfo Log: {}\n Filepath Vertex Shader: {}\n Filepath Fragment Shader: {}", type, infoLog, m_pathVertex, m_pathFragment);
				return false;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				MNEMOSY_ERROR("SHADER_PROGRAM_LINKING_ERROR of type: {} \nInfo Log: {}\n Filepath Vertex Shader: {}\n Filepath Fragment Shader: {}", type, infoLog, m_pathVertex, m_pathFragment);
				return false;
			}
		}

		return true;
	}

} // !mnemosy::graphics