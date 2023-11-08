#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <stdio.h>

class GuiPanel
{
public:
	virtual ~GuiPanel() = default;
	virtual void Render() = 0;

	bool show = true;
};
