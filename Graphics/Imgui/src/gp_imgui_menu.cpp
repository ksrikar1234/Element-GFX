#include "gp_imgui_menu.hpp"

#include "QtImGui.h"
#include "imgui.h"

imgui_menu::imgui_menu()
{
    
}

imgui_menu::~imgui_menu()
{

}

void imgui_menu::render()
{
    ImGui::Begin("Grid Sheet Viewer");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
}
