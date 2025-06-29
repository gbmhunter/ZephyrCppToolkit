#include "Mutex.hpp"

namespace zephyr_cpp_toolkit {

Mutex::Mutex()
{
    // Create Zephyr mutex
    k_mutex_init(&m_mutex);
}

Mutex::~Mutex()
{
}

} // namespace zephyr_cpp_toolkit