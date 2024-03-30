#ifndef TEXTURE_GENERATION_MANAGER_H
#define TEXTURE_GENERATION_MANAGER_H

#include <filesystem>

namespace fs = std::filesystem;

namespace mnemosy::graphics
{
	class Material;
	class Shader;
}

namespace mnemosy::systems
{
	
	class TextureGenerationManager {


	public:
		TextureGenerationManager();
		~TextureGenerationManager();

		/// <summary> FlipNormalMap() 
		/// Method to invert the y channel of the normal map texture of the material passed as a parameter. Texture is the exported as ktx to the specified path
		/// </summary>
		void FlipNormalMap(const char* exportPath, graphics::Material& material);

	private:
		bool IsInitialized();
		void InitializeShaderTextureAndFBO(unsigned int width,unsigned int height);

		unsigned int m_VBO = 0;
		unsigned int m_VAO = 0;
		
		unsigned int m_FBO = 0;
		unsigned int m_renderTexture_ID = 0;

		graphics::Shader* m_pTextureGenShader = nullptr;
		
		float m_screenQuadVertices[24] = {
			// triangle 1
			// positions xy		uvs
			-1.0f, -1.0f,		0.0f,0.0f,
			 1.0f,  1.0f,		1.0f,1.0f,
			-1.0f,  1.0f,		0.0f,1.0f,
			 // triangle 2
			 -1.0f, -1.0f,		0.0f,0.0f,
			  1.0f, -1.0f,		1.0f,0.0f,
			  1.0f,  1.0f,		1.0f,1.0f
		};
	};

} // !mnemosy::systems


#endif // !TEXTURE_GENERATION_MANAGER_H
