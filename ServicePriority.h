#pragma once
#include <cstdint>

/**
 * @enum ServicePriority
 * @brief Приоритет обслуживания устройства
 */

enum class ServicePriority : uint8_t {
	None = 0, ///< Обслуживание не требуется
	Low = 1, ///< Низкий приоритет обслуживания
	High = 2 ///< Высокий приоритет обслуживания
};
