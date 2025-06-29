#pragma once

#include <zephyr/kernel.h>

namespace zephyr_cpp_toolkit {

class Mutex {
public:
    Mutex();
    ~Mutex();
private:
    struct k_mutex m_mutex;
};

} // namespace zephyr_cpp_toolkit