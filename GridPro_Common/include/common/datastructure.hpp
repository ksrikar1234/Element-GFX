#ifndef DATASTRUCTURE_HPP
#define DATASTRUCTURE_HPP
#include <string>
#include <map>
#include <vector>
#include <set>

//#include <QString>

class TreeRowWidget;

namespace MainWindowActions
{
    enum ACTION{NONE, STATE, DIALOG, ACTION};
}
struct TreeItemSignature{
    enum TYPE{STATE, DIALOG};
    std::string modulename;
    std::string upperlevel;
    std::string itemname;
    std::string buttonname;
    std::string hide;
    int button_id;
    int level;
    TreeRowWidget *widget;
    int action_type;
    std::string action;
    std::string data;
    std::string item_id;

};

// struct ActionPacket
// {
//     enum WHO{UI, CONTROLLER};
//     int from;
//     int to;
//     QString action;
//     std::map<QString, QString> data;

//     void push(QString key, QString item)
//     {
//         data[key] = item;
//     }
// };



struct HistogramData{
    std::vector<double> values;
    std::vector<double> sample_distribution;
};

  struct MODULE
    { 
        std::string module_name;
        std::string module_icon;
        std::map <int, std::vector<std::string>> stages;
    };

 struct GLCOLORS
 {
    int hover[4]   = {255,255,255, 255};
    int surface[4] = {150,150,150,255};
    int cb_blue[4] = {0, 114, 178, 255};
    int cb_orange[4]  = {230, 159, 0, 255};
    int cb_sky_blue[4]  = {86, 180, 233, 255};
    int cb_yellow[4]  = {240, 228, 66, 255};
    int cb_green[4]  = {0, 158, 115, 255};
    int cb_vermillion[4]  = {213, 94, 0, 255};
    // int cb_purple = {204, 121, 167,255};
    // int cb_gray = {153, 153, 153,255};
    // int cb_cyan = {0,191,255,255};


 };

namespace GPGUI{
    enum PICKMODE {GP_PICK_SINGLE, GP_PICK_MULTIPLE};
    enum showlayers{SHOW_SURFACE = 1, SHOW_CAD = 2, SHOW_TOPOLOGY = 4, SHOW_GRID = 8};
}

enum GL_LAYERS{GP_SURFACE_GL_LAYER = 2, GP_CAD_GL_LAYER = 3, GP_TOPOLOGY_GL_LAYER = 4, GP_GRID_GL_LAYER = 5 };
 struct GP_ModuleSetup
    {
        std::string type;
        std::string settings_widget;
        std::string topology_path;
        std::string execution_id;
        std::string command;
        std::vector<std::string> parts_names;
        std::map<std::string, std::string> parts_ids;
        std::vector<std::string> parts_icons;
        std::map<std::string, std::string> parts_actions;
        std::map<std::string, int> parts_group;
        std::set<std::string> mandatory_fields;




    };
#endif // DATASTRUCTURE_HPP
