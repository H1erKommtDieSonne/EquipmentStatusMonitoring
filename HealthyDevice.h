#pragma once
#include "Device.h"
#include <memory>
#include <string>
#include <sstream>

/**
* @class HealthyDevice
* @brief Исправное устройство(наследник @ref Device)
* Дополнительно хранит наработку в секундах(@ref uptimeSec_) и
* предоставляет геттер @ref uptime.
* Поддерживает переходы:
*-@ref breakDown — перевод в неисправно
*-@ref repair — ремонт(создание нового исправного устройства)
*/

class HealthyDevice : public Device {
public:
    /**
  * @brief Конструктор исправного устройства.
  * @param name Имя устройства
  * @param addr IPадрес
  * @param prio Приоритет обслуживания
  * @param uptimeSec Наработка в секундах
  */
    HealthyDevice(std::string name, Address addr, ServicePriority prio, uint64_t uptimeSec)
        : Device(std::move(name), addr, prio), uptimeSec_(uptimeSec) {
    }

    /**
  * @brief Признак неисправности.
  * @return Всегда false для исправного устройства
  */

    bool isFaulty() const noexcept override { return false; }

    /**
     * @brief Текущая наработка устройства
     * @return Наработка в секундах
     */
    uint64_t uptime() const noexcept { return uptimeSec_; }

    /**
  * @brief Перевести устройство в состояние неисправно
  * @param fault описание причины поломки
  * @return Указатель на новое устройство в состоянии неисправно
  */
    std::unique_ptr<Device> breakDown(std::string fault) const override;

    /**
    * @brief Ремонт (создание нового исправного устройства)
    * @param uptimeAfterRepairSec Наработка после ремонта
    * @return Указатель на новое исправное устройство
    */
    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const override;

    /**
    * @brief Установить приоритет обслуживания
    * @param p Новое значение приоритета
    */
    void setPriority(ServicePriority p) override { priority_ = p; }

   
    std::string toString() const override {
        std::ostringstream os;
        os << "HealthyDevice{name=" << name_
            << ", addr=" << address_
            << ", prio=" << static_cast<int>(priority_)
            << ", uptimeSec=" << uptimeSec_ << "}";
        return os.str();
    }

protected:
    uint64_t uptimeSec_{ 0 };
};
