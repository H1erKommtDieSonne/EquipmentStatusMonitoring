#pragma once
/**
 * @file DeviceCollection.h
 * @brief Полиморфная коллекция устройств без слайсинга.
 */

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include "Device.h"
#include "ServicePriority.h"

 /**
  * @class DeviceCollection
  * @brief Хранилище устройств по указателям владения
  */
class DeviceCollection {
public:
    using Ptr = std::unique_ptr<Device>;

    /// @brief Добавить готовый объект
    void add(Ptr d) { devices_.emplace_back(std::move(d)); }

    /// @brief Создать устройство на месте и вернуть ссылку
    template <class T, class... Args>
    T& emplace(Args&&... args) {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        auto& ref = *p;
        devices_.emplace_back(std::move(p));
        return ref;
    }

    /// @brief Количество устройств
    std::size_t size() const noexcept { return devices_.size(); }

    /// @brief Найти по адресу
    Device* find(Device::Address addr) const noexcept {
        auto it = std::find_if(devices_.begin(), devices_.end(),
            [&](auto const& up) { return up->address() == addr; });
        return (it == devices_.end() ? nullptr : it->get());
    }

    /**
     * @brief Отсортировать по убыванию приоритета, затем по имени, затем по адресу
     */
    void sort_by_priority_desc() {
        std::sort(devices_.begin(), devices_.end(),
            [](auto const& a, auto const& b) {
                const int ra = priority_order(a->priority());
                const int rb = priority_order(b->priority());
                if (ra != rb) return ra > rb;
                if (a->name() != b->name()) return a->name() < b->name();
                return a->address() < b->address();
            });
    }

    /// @brief Доступ к контейнеру.
    const std::vector<Ptr>& data() const noexcept { return devices_; }

private:
    std::vector<Ptr> devices_;
};
