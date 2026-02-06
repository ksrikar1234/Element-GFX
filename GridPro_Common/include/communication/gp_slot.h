#ifndef GP_COMMUNICATION_SLOT_H
#define GP_COMMUNICATION_SLOT_H

#include <functional>
#include <string>
#include <memory>

namespace gridpro_comms
{
    class info_packet;

    class slot
    {
    public:
        using slot_function_type = std::function<std::shared_ptr<info_packet>(const std::shared_ptr<info_packet> &)>;

        slot() = default;

        explicit slot(slot_function_type func, uint32_t id, std::string name);

        slot(const slot &other);

        slot(slot &&other) noexcept;

        slot &operator=(const slot &other);

        slot &operator=(slot &&other) noexcept;

        std::shared_ptr<info_packet> call(const std::shared_ptr<info_packet> &packet) const;

        uint32_t get_id() const
        {
            return m_id;
        }

        std::string get_name() const
        {
            return m_name;
        }

    private:
        slot_function_type m_slot_function;
        uint32_t m_id;
        std::string m_name;
    };
}
#endif // GP_COMMUNICATION_SLOT_H
