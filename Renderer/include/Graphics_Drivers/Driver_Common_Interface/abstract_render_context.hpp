#ifndef _GP_ABSTRACT_RENDER_CONTEXT_H_
#define _GP_ABSTRACT_RENDER_CONTEXT_H_

namespace GridPro_GFX
{

class render_context 
{ 
    public:
    explicit render_context() : m_render_context_id(generate_context_id()) {}
    
    unsigned int id() const       { return m_render_context_id; }

    operator unsigned int() const { return m_render_context_id; }

    private :
    const unsigned int m_render_context_id;
    static unsigned int generate_context_id()
    {
        static unsigned int s_context_id = 0;
        return s_context_id++;
    }
};

}


#endif // _GP_ABSTRACT_RENDER_CONTEXT_H_