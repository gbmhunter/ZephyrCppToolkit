#include <zephyr/kernel.h>

#include "IPwm.hpp"

namespace zct {

IPwm::IPwm(const char* name) 
    : m_name(name)
{
    // Make sure the name is not null
    __ASSERT_NO_MSG(name != nullptr);
}

} // namespace zct