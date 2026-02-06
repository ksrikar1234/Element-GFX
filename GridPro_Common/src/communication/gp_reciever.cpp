#include "communication/gp_reciever.h"
#include "communication/gp_publisher.h"
#include "communication/gp_slot.h"
#include "communication/gp_packet.h"
#include <stdexcept>

namespace gridpro_comms
{

    generic_receiver::generic_receiver(const std::string &name)
        : m_name(name)
    {
        add_receiver(name);
    }

    generic_receiver::~generic_receiver()
    {
        publisher::instance()->remove_receiver(m_name);
    }

    void generic_receiver::add_receiver(const std::string &reciever_name)
    {
        if (reciever_name.empty())
        {
            printf("Warning !!!! Receiver name cannot be empty!!\n");
            return;
        }
        m_name = reciever_name;
        publisher::instance()->add_receiver(m_name, *this);
    }

    std::string generic_receiver::get_name()
    {
        return m_name;
    }

    void generic_receiver::remove_receiver()
    {
        publisher::instance()->remove_receiver(m_name);
    }

    slot *generic_receiver::get_slot(const std::string &slot_name)
    {
        auto it = m_slots.find(slot_name);
        if (it != m_slots.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    std::shared_ptr<info_packet> generic_receiver::process_packet(const std::shared_ptr<info_packet> &packet)
    {
        auto it = m_slots.find(packet->slot_name());
        if (it != m_slots.end())
        {
            return it->second.call(packet);
        }
        else
        {
            std::cout << "Slot : " << packet->slot_name() << "not found in :" << m_name << std::endl;
        }
    }

    std::shared_ptr<info_packet> generic_receiver::post_packet(const std::string &receiver_id, const std::string &message, std::map<QString, QString> &data)
    {
        data["sender_id"] = m_name.c_str();
        return gridpro_comms::post_packet(receiver_id, message, data);
    }

    std::shared_ptr<info_packet> generic_receiver::post_command(const std::string &receiver_id, const std::string &command)
    {
        auto p = gridpro_comms::create_packet(receiver_id, command);
        return post_packet(p);
    }

    std::shared_ptr<info_packet> generic_receiver::post_packet(const std::string &receiver_id, const std::string &message, std::map<QString, QString> &&data)
    {
        data["sender_id"] = m_name.c_str();
        return gridpro_comms::post_packet(receiver_id, message, data);
    }
    
    std::shared_ptr<info_packet> generic_receiver::send_signal(const std::string &receiver_id, const std::string &message, std::map<QString, QString> &&data)
    {
       return post_packet(receiver_id, message, std::move(data));        
    }
    
    std::shared_ptr<info_packet> generic_receiver::send_signal(const std::string &receiver_id, const std::string &message, std::map<QString, QString> &data)
    {
        return post_packet(receiver_id, message, data);
    }
    
    std::shared_ptr<info_packet> generic_receiver::post_packet(const std::shared_ptr<info_packet> &packet)
    {
        packet -> set_sender(m_name.c_str());
        return gridpro_comms::post(packet);
    }

    void generic_receiver::print_slots()
    {
        std::cout << "   \nReceiver Name : " << m_name << "\n   | Slots | : \n";
        for (auto &slot : m_slots)
        {
            std::cout << "     --> [" << slot.first << "]\n";
        }
    }

    uint32_t generic_receiver::generate_slot_id()
    {
        static uint32_t slot_id = 0;
        return ++slot_id;
    }

}