#include "Application/Include/GuiPanels/SceneSettingsGuiPanel.h"

#include "Application/Include/Application.h"

#include "Engine/Include/Graphics/Scene.h"
#include "Engine/Include/Graphics/Camera.h"
#include "Engine/Include/Graphics/Light.h"
#include "Engine/Include/Graphics/Skybox.h"
#include "Engine/Include/Graphics/RenderMesh.h"

#include "Engine/Include/Graphics/Renderer.h"
//#include "Engine/Include/Core/Window.h"

#include "Engine/External/ImGui/imgui.h"

#include <glm/glm.hpp>
#include <iostream>

namespace mnemosy::gui
{
	SceneSettingsGuiPanel::SceneSettingsGuiPanel()
	{
		panelName = "Scene Settings";
	}



	void SceneSettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;


		graphics::Scene& scene = ENGINE_INSTANCE().GetScene();
		graphics::Renderer& renderer = ENGINE_INSTANCE().GetRenderer();

		ImGui::Begin(panelName.c_str(), &showPanel);
		

		//ImGui::SeparatorText("Scene Settings");


		// Render Mesh settings
		if (ImGui::TreeNode("Mesh Settings"))
		{
			graphics::RenderMesh& renderMesh = scene.GetMesh();
			
			//ImGui::Text("Mesh Settings");

			float meshPos[3] = { 1.0f,1.0f,1.0f };
			meshPos[0] = renderMesh.transform.GetPosition().x;
			meshPos[1] = renderMesh.transform.GetPosition().y;
			meshPos[2] = renderMesh.transform.GetPosition().z;
			ImGui::DragFloat3("Mesh Position", (float*)meshPos, 0.1f, -100, 100, "%0.1f");
			glm::vec3 newMeshPos = glm::vec3(meshPos[0], meshPos[1], meshPos[2]);
			renderMesh.transform.SetPosition(newMeshPos);

			float meshRot[3] = { 1.0f,1.0f,1.0f };
			meshRot[0] = renderMesh.transform.GetRotationEulerAngles().x;
			meshRot[1] = renderMesh.transform.GetRotationEulerAngles().y;
			meshRot[2] = renderMesh.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Mesh Rotation", (float*)meshRot, 0.1f, -360.0f, 360.0f, "%0.1f");
			glm::vec3 newMeshRot = glm::vec3(meshRot[0], meshRot[1], meshRot[2]);
			renderMesh.transform.SetRotationEulerAngles(newMeshRot);


			ImGui::TreePop();
		}

		// Skybox Settings
		if (ImGui::TreeNode("Skybox Settings"))
		{

			graphics::Skybox& skybox = scene.GetSkybox();

			//ImGui::Text("Skybox Settings");
			ImGui::SliderFloat("Exposure", &skybox.exposure, -5.0f, 5.0f, "%.1f");
			ImGui::SliderFloat("Rotation", &skybox.rotation, 0.0f, 6.28f, "%1f");

			ImVec4 colorTint = ImVec4(0.0,0.0,0.0,1.0);
			colorTint.x = skybox.colorTint.r;
			colorTint.y = skybox.colorTint.g;
			colorTint.z = skybox.colorTint.b;
			ImGui::ColorEdit3("Color Tint", (float*)&colorTint);
			skybox.colorTint.r = colorTint.x;
			skybox.colorTint.g = colorTint.y;
			skybox.colorTint.b = colorTint.z;
		
			renderer.SetShaderSkyboxUniforms();

			ImGui::TreePop();
		}

		// Light Settings
		if (ImGui::TreeNode("Light Settings"))
		{
			graphics::Light& light = scene.GetLight();

			//ImGui::Text("Light Settings");
			const char* lightTypes_List[] = { "Directional", "Point" }; // they need to be ordered the same as in lightType Enum in light class
			static int lightType_current = light.GetLightTypeAsInt();

			ImGui::Combo("Light Type", &lightType_current, lightTypes_List, IM_ARRAYSIZE(lightTypes_List));

			if (light.GetLightTypeAsInt() != lightType_current)
			{
				light.SetType((graphics::LightType)lightType_current);
			}

		
			float pos[3] = { 1.0f,1.0f,1.0f };
			pos[0] = light.transform.GetPosition().x;
			pos[1] = light.transform.GetPosition().y;
			pos[2] = light.transform.GetPosition().z;
			ImGui::DragFloat3("Position", (float*)pos, 0.1f, -100, 100, "%0.1f");
			glm::vec3 newPos = glm::vec3(pos[0], pos[1], pos[2]);
			light.transform.SetPosition(newPos);


			float rot[3] = { 1.0f,1.0f,1.0f };
			rot[0] = light.transform.GetRotationEulerAngles().x;
			rot[1] = light.transform.GetRotationEulerAngles().y;
			rot[2] = light.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Rotation", (float*)rot, 0.1f, -360.0f, 360.0f, "%0.1f");
			glm::vec3 newRot = glm::vec3(rot[0], rot[1], rot[2]);
			light.transform.SetRotationEulerAngles(newRot);

			ImGui::DragFloat("Strength",&light.strength,1.0f,0.0f,1000.0f,"%.1f");
			ImGui::SliderFloat("Falloff",&light.falloff,0.01f,5.0f,"%.1f");
			ImVec4 lightColor = ImVec4(0.0, 0.0, 0.0, 1.0);
			lightColor.x = light.color.r;
			lightColor.y = light.color.g;
			lightColor.z = light.color.b;
			ImGui::ColorEdit3("Color", (float*)&lightColor);
			light.color.r = lightColor.x;
			light.color.g = lightColor.y;
			light.color.b = lightColor.z;

			renderer.SetPbrShaderLightUniforms();

			ImGui::TreePop();
		}

		// Camera Settings
		if (ImGui::TreeNode("Camera Settings"))
		{
			graphics::Camera& camera = scene.GetCamera();

			ImGui::SliderFloat("Field of View", &camera.settings.fov, 1.0f, 160.0f, "%.1f");

			ImGui::DragFloat("Near Clip",&camera.settings.nearClip , 0.1f, 0.00001, 100.0f, "%0.01f");
			ImGui::DragFloat("Far Clip",&camera.settings.farClip , 0.1f, 0.02, 5000.0f, "%0.01f");


			ImGui::TreePop();
		}

		// Post Processing Settings
		if (ImGui::TreeNode("Post processing"))
		{

			ImGui::Text("Here will go Post Processing settings");

			ImGui::TreePop();
		}

		//ImGui::ColorEdit3("MyColor##1", (float*)&color, misc_flags);

		ImGui::End();
	}

}