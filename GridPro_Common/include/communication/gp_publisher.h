#ifndef GP_COMMUNICATION_PUBLISHER_H
#define GP_COMMUNICATION_PUBLISHER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <future>
#include <map>

class QString;

namespace gridpro_comms
{

    class info_packet;
    class generic_receiver;

    /// @brief Publisher class
    /// @details This class is a singleton class that handles the broadcasting of packets to the receivers
    /// @note This class is a singleton class

    class publisher
    {
    public:
        using reciever_name_type  = std::string;
        using reciever_class_type = generic_receiver;

        static publisher* instance();

        void add_receiver(const reciever_name_type& name, reciever_class_type& receiver);

        void remove_receiver(const reciever_name_type& name);

        std::shared_ptr<info_packet> send(const std::shared_ptr<info_packet>& packet);

        std::future<std::shared_ptr<info_packet>> send_async(const std::shared_ptr<info_packet>& packet)
        {
            return std::async(std::launch::async, &publisher::send, this, packet);
        }

        void print_receivers();

    private:
         publisher() = default;
        ~publisher() = default;

         publisher(const publisher &) = delete;
         publisher &operator=(const publisher &) = delete;
         publisher(const publisher &&) = delete;
         publisher &operator=(const publisher &&) = delete;

         std::unordered_map<reciever_name_type, reciever_class_type*> m_receivers;
    };

    std::shared_ptr<info_packet> post(const std::shared_ptr<info_packet>& packet);
    std::shared_ptr<info_packet> post_command(const std::string &receiver_id, const std::string& command);

    std::future<std::shared_ptr<info_packet>> post_async(const std::shared_ptr<info_packet>& packet);

    std::shared_ptr<info_packet> post_packet(const std::string &receiver_id, const std::string& message, const std::map<QString, QString>& data);

    std::future<std::shared_ptr<info_packet>> post_packet_async(const std::string &receiver_id, const std::string& message, const std::map<QString, QString>& data);

    void print_connections();

}

#endif // GP_COMMUNICATION_PUBLISHER_H
