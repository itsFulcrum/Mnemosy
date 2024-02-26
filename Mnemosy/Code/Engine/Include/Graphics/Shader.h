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
		~Shader();

		void CreateShaderProgram(const char* vertexPath, const char* fragmentPath);

		void Use();

		void SetUniformBool(const std::string& name, bool value) const;

		void SetUniformInt(const std::string& name, int value) const;

		void SetUniformFloat(const std::string& name, float value) const;
		void SetUniformFloat2(const std::string& name, float x, float y) const;
		void SetUniformFloat3(const std::string& name, float x, float y, float z) const;
		void SetUniformFloat4(const std::string& name, float x, float y, float z, float w) const;
		void SetUniformMatrix4(const std::string& name, glm::mat4 matrix);

		void DeleteShaderProgram();

	private:
		std::string m_pathVertex;
		std::string m_pathFragment;


		void CheckCompileErrors(unsigned int shader, std::string type);

	};

} // mnemosy::graphics

#endif // !SHADER_H
