#ifndef _GP_IMGUI_GRID_VIEWER_HPP_
#define _GP_IMGUI_GRID_VIEWER_HPP_

#include "gp_imgui_widget.hpp"

#ifndef OLD_WS
#include "communication/gp_reciever.h"
#include "communication/gp_publisher.h"
#include "communication/gp_packet.h"
#endif

class imgui_grid_viewer : public imgui_widget, public gridpro_comms::generic_receiver
{
public:
    imgui_grid_viewer();
   ~imgui_grid_viewer() override;
    
    std::shared_ptr<gridpro_comms::info_packet>  packet_listener(const std::shared_ptr<gridpro_comms::info_packet> & packet);

    void render() override;
};

#endif
