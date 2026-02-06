#pragma once
#ifndef GP_INFO_PACKET_H
#define GP_INFO_PACKET_H

#include <common/datastructure.hpp>

#include <QPoint>
#include <QString>

#include <string>
#include <memory>
#include <map>
#include <any>
#include <optional>

namespace gridpro_comms
{
    /// @brief info_packet class
    /// Base class for all packets
    /// This class is the standard packet class for all the communication between the GridPro modules.
    class info_packet
    {
    public:
        /// @brief Constructors
        explicit info_packet(const std::string &receiver_id, const std::string &slot_name, const std::string &message);

        info_packet(const info_packet &other);

        info_packet(info_packet &&other) noexcept;

        static std::shared_ptr<info_packet> create(const std::string &receiver_id, const std::string &slot_name, const std::string &message);
        static std::shared_ptr<info_packet> create(const std::string &receiver_id, const std::string &message);

        /// @brief Destructor
        virtual ~info_packet() = default;

        /// @brief Assignment operators
        /// @param other
        /// @return
        info_packet &operator=(const info_packet &other);

        info_packet &operator=(info_packet &&other) noexcept;

        bool operator==(const info_packet &other) const;

        bool operator!=(const info_packet &other) const;

        /// @brief index operator
        QString& operator[](const QString &key);

        /// @brief Getters
        std::string receiver_id() const;

        std::string slot_name() const;

        std::string command() const;

        std::string sender() const;

        void set_sender(const std::string &name)
        {
            m_sender = name;
        }
       
        std::shared_ptr<std::map<QString, QString>>& data();

        std::shared_ptr<std::map<std::string, std::any>>& any_objects() { return m_any_objects; }

        QString get(const QString & key);

        bool contains(const QString & key);

        template <typename T, typename... Args>
        void set_data(const std::string &obj_name, Args &&...args)
        {
            try
            {
               ((*m_any_objects)[obj_name]).emplace<T>(std::forward<Args>(args)...);
            }
            catch (const std::exception &e)
            {
                printf("Error while loading object: %s \n", e.what());
            }
        }

        template <typename T>
        std::optional<T> get_data(const std::string& object_key)
        {
            auto it = m_any_objects->find(object_key);

            if (it == m_any_objects->end())
            {
                printf("No Object found with name %s", object_key.c_str());
                return std::nullopt;
            }
            else
            {
                try
                {
                    T& obj_ref = std::any_cast<T&>(m_any_objects->at(object_key));
                    return std::make_optional(obj_ref);
                }
                catch (const std::exception &e)
                {
                    printf("Error while recovering object: %s \n", e.what());
                    std::string type_name = typeid(T).name();
                    printf("Type Name : %s is not compatible with the object stored in the packet\n", type_name.c_str());
                    return std::nullopt;
                }
            }
        }

       
        std::any& any_type(std::string& object_name)
        {
            return (*m_any_objects)[object_name];
        }

        void print();

    private:
        std::string m_receiver_id;
        std::string m_slot_name;
        std::string m_command;
        std::string m_sender;
        std::shared_ptr<std::map<QString, QString>> m_data;
        std::shared_ptr<std::map<std::string, std::any>> m_any_objects;
    };

    using gp_packet = std::shared_ptr<info_packet>;

    std::shared_ptr<info_packet> create_packet(const std::string &receiver_id, const std::string &slot_name, const std::string &message);
    std::shared_ptr<info_packet> create_packet(const std::string &receiver_id, const std::string &message);


} // namespace gridpro_comms

#endif // GP_INFO_PACKET_H
