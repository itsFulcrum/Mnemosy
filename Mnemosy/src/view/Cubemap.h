#ifndef CUBEMAP_H
#define CUBEMAP_H

#pragma once
#include <glad/glad.h>
#include <iostream>
#include <vector>


#include<stb_image.h>


#include <view/Shader.h>
#include <model/Object.h>


class Cubemap
{
public:
	unsigned int colorCubemapID = NULL;
	unsigned int irradianceMapID = NULL;
	unsigned int prefilterMapID = NULL;
	unsigned int brdfLUTTextureID = NULL;

private:
	unsigned int equirectangularImageID;
	bool m_hasAlphaChannel = false;
	int m_channelsAmount = 0;

public:

	Cubemap(std::string path, unsigned int textureResolution, bool GenerateConvolutedMaps)
	{

		// generate normal opengl texture from the hdri panorama
		glGenTextures(1, &equirectangularImageID);
		glBindTexture(GL_TEXTURE_2D, equirectangularImageID);

		stbi_set_flip_vertically_on_load(false);

		int width, height, channelsAmount;
		// replace paths list with normal string not list of strings
		//unsigned char* panoramaImageData = stbi_load(path.c_str(), &width, &height, &channelsAmount, 0);
		// loading as float to preserve full dynamic range
		float* panoramaImageData = stbi_loadf(path.c_str(), &width, &height, &channelsAmount, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, panoramaImageData);


		if (!panoramaImageData)
		{
			std::cout << "ERROR::CUBEMAP:: COULD_NOT_LOAD_IMAGE at path: " << path << std::endl;
			stbi_image_free(panoramaImageData);
			return;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(panoramaImageData);
		
		
		
		Shader equirectangularToCubemapShader("src/shaders/equirectangularToCubemap.vert", "src/shaders/equirectangularToCubemap.frag");
		Shader brdfLUTShader("src/shaders/equirectangularToCubemap.vert", "src/shaders/cookTorranceBRDFLut.frag");
		Object unitCube("fbx/skyboxMesh.fbx");

		
		equirectangularToCubemap(colorCubemapID,textureResolution,false, equirectangularToCubemapShader, unitCube);
		

		if (GenerateConvolutedMaps) 
		{
			// irradiance map
			equirectangularToCubemap(irradianceMapID, 32, true, equirectangularToCubemapShader, unitCube);
			// prefilter Map
			equirectangularToPrefilteredCubemap(prefilterMapID, 128, equirectangularToCubemapShader, unitCube);
			// brdf Lut Texture // we could easily save this out on disk as it will not change unless we change lighting formulas
			generateBrdfLUTTexture(brdfLUTShader, unitCube);
		}
		
	
	};

	// equirectangularToCubemap Shader has 3 modes configuragble via uniform int "_mode"
	// 0 = crate normal cubemap from equirectangular map
	// 1 = create a convoluted irradiance map from an equirectangular map
	// 2 = create a prefilter convoluted map from an equirectangular map // this reqiuires also a roughness value set via "_roughness"

	void equirectangularToCubemap(unsigned int& cubemapID,unsigned int textureRes, bool makeConvolutedIrradianceMap, Shader& equirectangularToCubemapShader, Object& unitCubeObject)
	{
		// to create a cubemap from an equirectangular panorama Hdri image I Create a framebuffer that renders to a texture.
		// the using a clever shader by the khronos group we loop through all 6 sides of the cube/cubemap textures and render each directly to the cubemap object;
		// For this we need said shaders and a dummy object to make a drawcall but as i understad it it doesn't matter what the object is since it's all handled by the shader
		// we just need to make some drawcall
		
		
		glViewport(0, 0, textureRes, textureRes);


		// fill in filepath
		//Shader equirectangularToCubemapShader("src/shaders/equirectangularToCubemap.vert","src/shaders/equirectangularToCubemap.frag");
		//Object dummyObj("fbx/skyboxMesh.fbx");
		
		
		unsigned int fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		// create cubemap
		for (int i = 0; i < 6; ++i) 
		{
			auto data = std::vector<unsigned char>();
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, textureRes, textureRes, 0, GL_RGB, GL_FLOAT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


		

		for(int i = 0;i < 6;i++)
		{
			int side = i;

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, cubemapID, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "ERROR::CUBEMAP::FRAMEBUFFER_NOT_COMPLETE" << std::endl;
				
				glDeleteFramebuffers(1, &fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				
				return;
			}

			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
			
			glViewport(0, 0, textureRes, textureRes);
			
			glClearColor(1.0f, 0.0f, 1.0f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			

			equirectangularToCubemapShader.use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, equirectangularImageID);

			equirectangularToCubemapShader.setUniformInt("_equirectangularMap", 0);
			equirectangularToCubemapShader.setUniformInt("_currentFace", side);
			if(makeConvolutedIrradianceMap)
				equirectangularToCubemapShader.setUniformInt("_mode", 1);
			else
				equirectangularToCubemapShader.setUniformInt("_mode", 0);



			unitCubeObject.Render(equirectangularToCubemapShader, glm::mat4(0.0f), glm::mat4(0.0f));

		}


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		
		// return to default framebuffer
		glDeleteFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	}

	void equirectangularToPrefilteredCubemap(unsigned int& cubemapID, unsigned int textureRes, Shader& equirectangularToCubemapShader, Object& unitCubeObject)
	{
		// to create a cubemap from an equirectangular panorama Hdri image I Create a framebuffer that renders to a texture.
		// the using a clever shader by the khronos group we loop through all 6 sides of the cube/cubemap textures and render each directly to the cubemap object;
		// For this we need said shaders and a dummy object to make a drawcall but as i understad it it doesn't matter what the object is since it's all handled by the shader
		// we just need to make some drawcall

		unsigned int textureResolution = textureRes;
		glViewport(0, 0, textureResolution, textureResolution);


		unsigned int fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		// create cubemap
		for (int i = 0; i < 6; ++i)
		{
			auto data = std::vector<unsigned char>();
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, textureResolution, textureResolution, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		equirectangularToCubemapShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, equirectangularImageID);

		equirectangularToCubemapShader.setUniformInt("_equirectangularMap", 0);
		equirectangularToCubemapShader.setUniformInt("_mode", 2);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		unsigned int maxMipLevels = 5;

		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipRes = int(textureResolution * std::pow(0.5, mip));

			glViewport(0, 0, mipRes, mipRes);

			float roughness = float(mip) / float(maxMipLevels - 1);
			equirectangularToCubemapShader.setUniformFloat("_roughness", roughness);
		
			for (int i = 0; i < 6; i++)
			{
				int face = i;

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemapID, mip);

				// check if framebuffer is initialize correctly
				/*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				{
					std::cout << "ERROR::CUBEMAP::FRAMEBUFFER_NOT_COMPLETE" << std::endl;
					glDeleteFramebuffers(1, &fbo);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					return;
				}*/

				glClearColor(1.0f, 0.0f, 1.0f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				equirectangularToCubemapShader.setUniformInt("_currentFace", face);
				unitCubeObject.Render(equirectangularToCubemapShader, glm::mat4(0.0f), glm::mat4(0.0f));

			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		// return to default framebuffer
		glDeleteFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void generateBrdfLUTTexture(Shader brdfLUTShader,Object& unitCubeObject)
	{
		glGenTextures(1, &brdfLUTTextureID);

		int res = 512;
		glBindTexture(GL_TEXTURE_2D, brdfLUTTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, res, res, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		unsigned int fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTextureID, 0);

		glViewport(0, 0, res, res);

		glClearColor(1.0f, 0.0f, 1.0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		brdfLUTShader.use();

		unitCubeObject.Render(brdfLUTShader, glm::mat4(0.0f), glm::mat4(0.0f));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTextureID);
		glGenerateMipmap(GL_TEXTURE_2D);

		glDeleteFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	void BindColorCubemap(unsigned int location)
	{
		if (colorCubemapID != NULL)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, colorCubemapID);

		}
		else
		{
			std::cout << "ERROR::CUBEMAP::COLOR_MAP_IS_NOT_YET_GENERATED" << std::endl;
		}
	}
	void BindIrradianceCubemap(unsigned int location)
	{
		if(irradianceMapID != NULL)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapID);
			
		}
		else
		{
			std::cout << "ERROR::CUBEMAP::IRRADIANCE_MAP_IS_NOT_YET_GENERATED" << std::endl;
		}
	}
	void BindPrefilteredCubemap(unsigned int location)
	{
		if (prefilterMapID != NULL)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMapID);

		}
		else
		{
			std::cout << "ERROR::CUBEMAP::PREFILTERED_MAP_IS_NOT_YET_GENERATED" << std::endl;
		}
	}
	void BindBrdfLutTexture(unsigned int location)
	{
		if (brdfLUTTextureID != NULL)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, brdfLUTTextureID);

		}
		else
		{
			std::cout << "ERROR::CUBEMAP::BRDFLUT_MAP_IS_NOT_YET_GENERATED" << std::endl;
		}

	}

	int GetChannelsAmount()
	{
		return m_channelsAmount;
	}


};


#endif 
