#include <iostream>

#include "communication/gp_packet.h"
#include "communication/gp_publisher.h"
#include "communication/gp_reciever.h"

namespace gridpro_comms
{
    ///              API for the publisher class
    /// ********************************************************************************************************************
    /// @brief Create a packet and post it to appropriate receiver
    /// @param receiver_id  to whom the packet is to be sent
    /// @param slot_name  the slot name of the receiver to need be called
    /// @param message  the message to be sent
    /// @return  void
    /// @note This function creates a packet with default slot name "default_packet_listener"
    std::shared_ptr<info_packet> post_packet(const std::string &receiver_id, const std::string &message, const std::map<QString, QString> &data)
    {
        auto packet = info_packet::create(receiver_id, message);
        (packet->data()) = std::make_shared<std::map<QString, QString>>(data);
        return post(packet);
    }
    
    std::shared_ptr<info_packet> post_command(const std::string &receiver_id, const std::string& command)
    {
        auto packet = info_packet::create(receiver_id, command);
        (packet->data()) = std::make_shared<std::map<QString, QString>>();
        return post(packet);
    }
    /// @brief Post a packet asynchronously
    /// @param receiver_id  to whom the packet is to be sent
    /// @param slot_name  the slot name of the receiver to need be called
    /// @param message  the message to be sent
    /// @return  std::future<void>
    /// @note This function creates a packet with default slot name "default_packet_listener"
    std::future<std::shared_ptr<info_packet>> post_packet_async(const std::string &receiver_id, const std::string &message, const std::map<QString, QString> &data)
    {
        return std::async(std::launch::async, &gridpro_comms::post_packet, receiver_id, message, data);
    }

    /// @brief Post a packet
    /// @param packet
    std::shared_ptr<info_packet> post(const std::shared_ptr<info_packet> &packet)
    {
        return publisher::instance()->send(packet);
    }

    std::future<std::shared_ptr<info_packet>> post_async(const std::shared_ptr<info_packet> &packet)
    {
        return publisher::instance()->send_async(packet);
    }
    
    void print_connections()
    {
        publisher::instance()->print_receivers();
    }

    /// ********************************************************************************************************************
    ///                    API for the publisher class
    
    
    /// ***********************************Internal Class Member funtions **************************************************

    publisher *publisher::instance()
    {
        static publisher instance;
        return &instance;
    }

    void publisher::add_receiver(const reciever_name_type &name, reciever_class_type &receiver)
    {
        m_receivers[name] = &receiver;
        std::cout << "Added receiver: " << name << std::endl;
        std::cout << "Total receivers: " << m_receivers.size() << std::endl;
    }

    void publisher::remove_receiver(const reciever_name_type &name)
    {
        m_receivers.erase(name);
    }

    std::shared_ptr<info_packet> publisher::send(const std::shared_ptr<info_packet>& packet)
    {
        auto it = m_receivers.find(packet->receiver_id());
        if (it != m_receivers.end())
        {
            std::shared_ptr<info_packet> detached_packet = std::move(packet);
            return it->second->process_packet(detached_packet);
        }
        else
        {
            std::cout << "Receiver not found for packet: " << packet->receiver_id() << std::endl;
            return nullptr;
        }
    }
    
    void publisher::print_receivers()
    {
        std::cout << "[ ------------ Available Receivers -------------- ] \n\n";
        for (auto &receiver : m_receivers)
        {
            receiver.second->print_slots();
        }
        std::cout << "\n[ ----------------------------------------------- ]\n";
    }

}
