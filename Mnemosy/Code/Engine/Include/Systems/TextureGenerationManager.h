#ifndef TEXTURE_GENERATION_MANAGER_H
#define TEXTURE_GENERATION_MANAGER_H

#include <filesystem>

namespace fs = std::filesystem;

namespace mnemosy::graphics
{
	class Material;
	class Shader;
	enum ChannelPackType;
	enum ChannelPackComponent;
}

namespace mnemosy::systems
{
	
	class TextureGenerationManager {


	public:
		TextureGenerationManager();
		~TextureGenerationManager();

		// Method to invert the y channel of the normal map texture of the material passed as a parameter.
		void FlipNormalMap(const char* exportPath, graphics::Material& material,bool exportTexture);

		void InvertRoughness(graphics::Material& material, const char* exportPath, bool exportTexture);

		void GenerateOpacityFromAlbedoAlpha(graphics::Material& material, const char* exportPath, bool exportTexture);

		bool GenerateChannelPackedTexture(graphics::Material& material, const char* exportPath, bool exportTexture, graphics::ChannelPackType packType, graphics::ChannelPackComponent packComponent_R, graphics::ChannelPackComponent packComponent_G, graphics::ChannelPackComponent packComponent_B, graphics::ChannelPackComponent packComponent_A, unsigned int width, unsigned int height,uint8_t bitDepth);


		unsigned int GetRenderTextureID() { return m_renderTexture_ID; }


	private:
		bool IsInitialized();
		void InitializeShaderTextureAndFBO(unsigned int width,unsigned int height);

		void DrawQuad();

		graphics::Shader* m_pTextureGenShader = nullptr;

		unsigned int m_VBO = 0;
		unsigned int m_VAO = 0;
		
		unsigned int m_FBO = 0;
		unsigned int m_renderTexture_ID = 0;

		
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
