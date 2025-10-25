#pragma once
/**
 * @file ServicePriority.h
 * @brief Приоритет обслуживания устройств.
 */

#include <cstdint>
#include <string>
#include <compare>

 /**
  * @enum ServicePriority
  * @brief Приоритеты обслуживания (больше — важнее).
  *
  * Порядок важности: High > Low > None.
  */
enum class ServicePriority : std::uint8_t {
    None = 0,  ///< Обслуживание не требуется
    Low = 1,  ///< Низкий приоритет
    High = 2   ///< Высокий приоритет
};

/**
 * @brief Численный порядок для сравнения и сортировки.
 * @return 3 для High, 2 для Low, 1 для None.
 */
constexpr int priority_order(ServicePriority p) noexcept {
    switch (p) {
    case ServicePriority::High: return 3;
    case ServicePriority::Low:  return 2;
    case ServicePriority::None: return 1;
    }
    return 0;
}

/**
 * @brief Человекочитаемая строка для приоритета.
 */
inline std::string to_string(ServicePriority p) {
    switch (p) {
    case ServicePriority::High: return "High";
    case ServicePriority::Low:  return "Low";
    case ServicePriority::None: return "None";
    }
    return "Unknown";
}

/**
 * @brief Сравнение приоритетов: High > Low > None.
 * @details Фиксирует инвариант «бОльшая важность — больше по операторам сравнения».
 */
inline std::strong_ordering operator<=>(ServicePriority lhs, ServicePriority rhs) noexcept {
    const int a = priority_order(lhs);
    const int b = priority_order(rhs);
    if (a < b) return std::strong_ordering::less;
    if (a > b) return std::strong_ordering::greater;
    return std::strong_ordering::equivalent;
}
