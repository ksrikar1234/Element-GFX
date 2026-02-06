#include "communication/gp_slot.h"
#include "communication/gp_packet.h"

namespace gridpro_comms
{

    slot::slot(slot_function_type func, uint32_t id, std::string name)
        : m_slot_function(func), m_id(id), m_name(name)
    {
    }

    slot::slot(const slot &other)
        : m_slot_function(other.m_slot_function), m_id(other.m_id), m_name(other.m_name) {}

    slot::slot(slot &&other) noexcept
        : m_slot_function(std::move(other.m_slot_function)), m_id(other.m_id), m_name(std::move(other.m_name)) {}

    slot &slot::operator=(const slot &other)
    {
        if (this != &other)
        {
            m_slot_function = other.m_slot_function;
            m_id = other.m_id;
            m_name = other.m_name;
        }
        return *this;
    }

    slot &slot::operator=(slot &&other) noexcept
    {
        if (this != &other)
        {
            m_slot_function = std::move(other.m_slot_function);
            m_id = other.m_id;
            m_name = std::move(other.m_name);
        }
        return *this;
    }

    std::shared_ptr<info_packet> slot::call(const std::shared_ptr<info_packet> &packet) const
    {

        if (m_slot_function)
        {
            return m_slot_function(packet);
        }
    }

}