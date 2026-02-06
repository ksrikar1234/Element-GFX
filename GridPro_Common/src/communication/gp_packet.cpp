#include "communication/gp_packet.h"

namespace gridpro_comms
{
    /// ********************************************************************************************************************
    /// @brief Create a packet
    /// @param receiver_id  to whom the packet is to be sent
    /// @param slot_name  the slot name to be used
    /// @param message  the message to be sent
    /// @return  std::shared_ptr<info_packet>
    std::shared_ptr<info_packet> create_packet(const std::string &receiver_id, const std::string &slot_name, const std::string &message)
    {
        return info_packet::create(receiver_id, slot_name, message);
    
    }
    /// @brief Create a packet with default slot name
    /// @param receiver_id  to whom the packet is to be sent
    /// @param message  the message to be sent
    /// @return  std::shared_ptr<info_packet>
    /// @note This function creates a packet with default slot name "default_packet_listener"
    std::shared_ptr<info_packet> create_packet(const std::string &receiver_id, const std::string &message)
    {
        return create_packet(receiver_id, "default_packet_listener", message);
    }
    /// ********************************************************************************************************************
    
    /// @brief Constructors
    info_packet::info_packet(const std::string &receiver_id, const std::string &slot_name, const std::string &command)
        : m_receiver_id(receiver_id), m_slot_name(slot_name), m_command(command) 
    {
        m_data = std::make_shared<std::map<QString, QString>>();
        m_any_objects = std::make_shared<std::map<std::string, std::any>>();
    }

    info_packet::info_packet(const info_packet &other)
        : m_receiver_id(other.m_receiver_id), m_slot_name(other.m_slot_name), m_command(other.m_command),  m_data(other.m_data), m_any_objects(other.m_any_objects) {}

    info_packet::info_packet(info_packet &&other) noexcept
        : m_receiver_id(std::move(other.m_receiver_id)), m_slot_name(std::move(other.m_slot_name)), m_command(std::move(other.m_command)) , m_data(std::move(other.m_data)) , m_any_objects(other.m_any_objects){}

    std::shared_ptr<info_packet> info_packet::create(const std::string &receiver_id, const std::string &slot_name, const std::string &message)
    {
        return std::make_shared<info_packet>(receiver_id, slot_name, message);
    }

    std::shared_ptr<info_packet> info_packet::create(const std::string &receiver_id, const std::string &message)
    {
        return create(receiver_id, "default_packet_listener", message);
    }

    info_packet &info_packet::operator=(const info_packet &other)
    {
        if (this != &other)
        {
            m_receiver_id = other.m_receiver_id;
            m_slot_name = other.m_slot_name;
            m_command = other.m_command;
            m_data = other.m_data;
            m_any_objects = other.m_any_objects;
        }
        return *this;
    }

    info_packet &info_packet::operator=(info_packet &&other) noexcept
    {
        if (this != &other)
        {
            m_receiver_id = std::move(other.m_receiver_id);
            m_slot_name = std::move(other.m_slot_name);
            m_command = std::move(other.m_command);
            m_data = std::move(other.m_data);
            m_any_objects = other.m_any_objects;
        }
        return *this;
    }

    bool info_packet::operator==(const info_packet &other) const
    {
        return m_receiver_id == other.m_receiver_id && m_slot_name == other.m_slot_name && m_command == other.m_command && m_data == other.m_data && m_any_objects == other.m_any_objects;
    }

    bool info_packet::operator!=(const info_packet &other) const
    {
        return !(*this == other);
    }
    
    QString& info_packet::operator[](const QString &key)
    {
        if(m_data->find(key) == m_data->end())
        {
            (*m_data)[key] = "";
        }
        return (*m_data)[key];
    }

    /// @brief Getters
    std::string info_packet::receiver_id() const
    {
        return m_receiver_id;
    }

    std::string info_packet::slot_name() const
    {
        return m_slot_name;
    }

    std::string info_packet::command() const
    {
        return m_command;
    }

    std::string info_packet::sender() const
    {
        return m_sender;
    }


    QString info_packet::get(const QString & key)
    {
        if(m_data->find(key) == m_data->end())
          { 
            printf("Key : %s not found in the packet\n", key[0]);
            return "";
          }
        else
            return (*m_data)[key];
    }

    bool info_packet::contains(const QString & key)
    {
        if(m_data->find(key) == m_data->end())
            return false;
        else
            return true;
    }

    std::shared_ptr<std::map<QString, QString>>& info_packet::data()
    {
        return m_data;
    }

    void info_packet::print()
    {
        printf("Receiver ID : %s\n", m_receiver_id.c_str());
        printf("Slot Name : %s\n", m_slot_name.c_str());
        printf("Message : %s\n", m_command.c_str());
        for(auto& item : *m_data)
        {
            printf("%s : %s\n", item.first.toStdString().c_str(), item.second.toStdString().c_str());
        }
    }
}