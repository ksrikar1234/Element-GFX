#ifndef GP_GUI_ABSTRACT_RENDERING_SYSTEM_H
#define GP_GUI_ABSTRACT_RENDERING_SYSTEM_H

#include "ecs.h"

namespace gridpro_gui
{
   class Abstract_RenderDevice : public ecs::System
   {
     public :
     void init() override {}
     void update(float layer) override {} 
   };
}

#endif // GP_GUI_RENDERING_SYSTEM_H