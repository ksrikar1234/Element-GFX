#ifndef _GP_IMGUI_MENU_HPP_
#define _GP_IMGUI_MENU_HPP_

#include "gp_imgui_widget.hpp"

class imgui_menu : public imgui_widget
{
public:
    imgui_menu();
   ~imgui_menu() override;

    void render() override;
};

#endif
