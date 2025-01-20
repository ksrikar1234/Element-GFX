#include "gp_imgui_grid_viewer.hpp"

#include <QString>

#include "QtImGui.h"
#include "imgui.h"

#ifndef OLD_WS
#include "communication/gp_publisher.h"
#endif

std::string currentSheet = "";
int sheetDirection = 0;

std::map<int, std::string> sheet_map;

imgui_grid_viewer::imgui_grid_viewer() : gridpro_comms::generic_receiver("imgui_grid_viewer")
{
    add_packet_listener(this, &imgui_grid_viewer::packet_listener);
}

imgui_grid_viewer::~imgui_grid_viewer()
{
}

void DrawHighlightedText(const char *text, const ImVec4 &highlight_color)
{
    ImVec2 position = ImGui::GetCursorScreenPos();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Ensure default text color
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Get text size for background rectangle
    ImVec2 text_size = ImGui::CalcTextSize(text);

    // Draw the highlight rectangle
    draw_list->AddRectFilled(
        position,
        ImVec2((position.x + text_size.x) * 1.02, (position.y + text_size.y) * 1.02),
        ImGui::GetColorU32(highlight_color));

    // Draw the text on top
    ImGui::SetCursorScreenPos(position);
    ImGui::TextUnformatted(text);

    ImGui::PopStyleColor();
}

void imgui_grid_viewer::render()
{
    std::map<QString, QString> data;

    data["command"] = "load_grid_file";
    data["grid_name"] = "/home/gridpro-graphics-hpc/Downloads/blk.tmp";

    ImGui::Begin("Grid Viewer");

    ImGui::Text("Load Grid File");

    char grid_name[1024];

    for (int i = 0; i < 1024; i++)
    {
        grid_name[i] = '\0';
    }

    static std::string grid_name_str;

    if (ImGui::InputText("Grid Name", grid_name, IM_ARRAYSIZE(grid_name)))
    {
        grid_name_str = std::string(grid_name);
    }

    if (ImGui::Button("Load Grid"))
    {
        printf("Loading Grid \n");
        printf("Grid Nmae User Input: %s \n", grid_name_str.c_str());
        if (!grid_name_str.empty())
        {
            data["grid_name"] = grid_name_str.c_str();
            post_packet("grid_presenter", "load_grid_file", data);

            std::map<QString, QString> request_grid_sheet_list;
            request_grid_sheet_list["command"] = "get_current_sheet_list";
            post_packet("grid_presenter", "get_current_sheet_list", request_grid_sheet_list);
        }
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        DrawHighlightedText("Display Grid Sheet", ImVec4(0.0f, 0.2f, 0.2f, 1.0f));

        for (auto &sheet : sheet_map)
        {
            if (ImGui::Button(sheet.second.c_str()))
            {
                std::map<QString, QString> set_current_sheet_info;
                set_current_sheet_info["command"] = "set_current_sheet";
                set_current_sheet_info["sheet_name"] = sheet.second.c_str();
                post_packet("grid_presenter", "set_current_sheet", set_current_sheet_info);
                currentSheet = sheet.second;
            }
        }
        
        std::string curr_sheet_messg = "Current Sheet: " + currentSheet;

        DrawHighlightedText(curr_sheet_messg.c_str(), ImVec4(0.0f, 0.2f, 0.2f, 1.0f)); // Display the current sheet

        if (ImGui::Button("<"))
        {
            std::map<QString, QString> forward_sheet_info;
            forward_sheet_info["command"] = "decrement_current_sheet";
            post_packet("grid_presenter", "decrement_current_sheet", forward_sheet_info);
        }

        ImGui::SameLine();

        if (ImGui::Button(">"))
        {
            std::map<QString, QString> backward_sheet_info;
            backward_sheet_info["command"] = "increment_current_sheet";
            post_packet("grid_presenter", "increment_current_sheet", backward_sheet_info);
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    ImGui::End();
}

std::shared_ptr<gridpro_comms::info_packet>  imgui_grid_viewer::packet_listener(const std::shared_ptr<gridpro_comms::info_packet> &packet)
{
    if ((*packet)["command"] == "get_current_sheet_list")
    {
        sheet_map.clear();
        for (int i = 0; i < packet->data()->size(); i++)
        {
            QString key = "sheet_" + QString::number(i);
            if (packet->data()->find(key) != packet->data()->end())
            {
                sheet_map[i] = (*packet)[key].toStdString();
            }
            else
            {
                break;
            }
        }
    }
    return nullptr;
}