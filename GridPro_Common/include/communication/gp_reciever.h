#pragma once
#ifndef GP_COMMUNICATIONS_RECEIVER_H
#define GP_COMMUNICATIONS_RECEIVER_H

#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <iostream>
#include <functional>
#include "communication/gp_slot.h"

class QString;

namespace gridpro_comms
{
    class info_packet;
    // class slot;

    class generic_receiver
    {
    public:
        explicit generic_receiver(const std::string &name);
        virtual ~generic_receiver();

        std::string get_name();
        slot *get_slot(const std::string &slot_name);

        void add_receiver(const std::string &reciever_name);
        void remove_receiver();

        std::shared_ptr<info_packet> post_packet(const std::string &receiver_id, const std::string &message, std::map<QString, QString>&  data);
        std::shared_ptr<info_packet> post_packet(const std::string &receiver_id, const std::string &message, std::map<QString, QString>&& data);
        std::shared_ptr<info_packet> post_packet(const std::shared_ptr<info_packet> &packet);

        std::shared_ptr<info_packet> send_signal(const std::string &receiver_id, const std::string &message, std::map<QString, QString>& data);
        std::shared_ptr<info_packet> send_signal(const std::string &receiver_id, const std::string &message, std::map<QString, QString>&& data);

        std::shared_ptr<info_packet> post_command(const std::string &receiver_id, const std::string &command);
        /// @warning: This function is needs to be inlined always
        ///           because it is a template function
        template <typename T>
        void add_slot(const std::string &slot_name, T *instance, std::shared_ptr<info_packet> (T::*func)(const std::shared_ptr<info_packet> &))
        {
            uint32_t id = generate_slot_id();
            std::function<std::shared_ptr<info_packet>(const std::shared_ptr<info_packet> &)> slot_function = [instance, func](const std::shared_ptr<info_packet> &packet)
            {
                if (func)
                    return (instance->*func)(packet);
                else
                    return std::shared_ptr<info_packet>();
            };
            slot new_slot(slot_function, id, slot_name);
            m_slots[slot_name] = std::move(new_slot);
            return;
        }

        template <typename T>
        void add_packet_listener(T *instance, std::shared_ptr<info_packet> (T::*func)(const std::shared_ptr<info_packet> &))
        {
            add_slot("default_packet_listener", instance, func);
        }

        void print_slots();

    private:
        friend class publisher;
        uint32_t generate_slot_id();
        std::shared_ptr<info_packet> process_packet(const std::shared_ptr<info_packet> &packet);

        std::unordered_map<std::string, slot> m_slots;
        std::string m_name;
    };
} // namespace gridpro_comms

#endif // GP_COMMUNICATIONS_RECEIVER_H