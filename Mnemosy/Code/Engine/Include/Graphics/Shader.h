#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <string>

namespace mnemosy::graphics
{

	class Shader 
	{
	public:
		unsigned int ID = NULL;

		Shader(const char* vertexPath, const char* fragmentPath);
		Shader(const char* MeshPath, const char* fragmentPath, bool isMeshShader);
		~Shader();

		bool CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
		bool CreateMeshShaderProgramm(const char* meshPath, const char* fragmentPath);

		void Use();

		void SetUniformBool(const char* name, bool value) const;

		void SetUniformInt(const char* name, int value) const;

		void SetUniformFloat(const char* name, float value) const;
		void SetUniformFloat2(const char* name, float x, float y) const;
		void SetUniformFloat3(const char* name, float x, float y, float z) const;
		void SetUniformFloat4(const char* name, float x, float y, float z, float w) const;
		void SetUniformMatrix4(const char* name, const glm::mat4& matrix);

		void DeleteShaderProgram();

	private:

		std::string m_pathVertex;
		std::string m_pathFragment;


		bool CheckCompileErrors(unsigned int shader,const std::string& type);

	};

} // mnemosy::graphics

#endif // !SHADER_H
