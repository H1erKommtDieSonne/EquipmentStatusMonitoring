#pragma once
#include <cstdint>

/**
 * @enum ServicePriority
 * @brief Приоритет обслуживания устройства
 */
enum class ServicePriority : uint8_t { None = 0, Low = 1, High = 2 };
