#pragma once
/**
 * @file DeviceCollection.h
 * @brief Полиморфная коллекция устройств без слайсинга.
 *
 * коллекция дополнена деревом хэшей.
 * Владение устройствами остаётся у std::vector<std::unique_ptr<Device>>,
 * а DeviceHashTree хранит только невладеющие указатели Device*.
 */

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "Device.h"
#include "DeviceHashTree.h"
#include "ServicePriority.h"

 /**
  * @class DeviceCollection
  * @brief Хранилище устройств по указателям владения.
  *
  * Основной контейнер:
  * std::vector<std::unique_ptr<Device>>
  *
  * Дополнительный индекс:
  * DeviceHashTree
  *
  * Индекс ускоряет поиск устройства по сетевому адресу.
  */
class DeviceCollection {
public:
    using Ptr = std::unique_ptr<Device>;

    /**
     * @brief Добавить готовый объект.
     * @param d Указатель владения на устройство.
     *
     * Устройство помещается в основной vector, а его обычный указатель
     * добавляется в дерево хэшей.
     */
    void add(Ptr d) {
        Device* raw = d.get();

        devices_.emplace_back(std::move(d));

        if (raw != nullptr) {
            addressIndex_.insert(raw);
        }
    }

    /**
     * @brief Создать устройство на месте и вернуть ссылку.
     *
     * Метод удобен тем, что объект сразу создаётся внутри коллекции.
     * После добавления его адрес также заносится в дерево хэшей.
     */
    template <class T, class... Args>
    T& emplace(Args&&... args) {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        auto& ref = *p;

        devices_.emplace_back(std::move(p));
        addressIndex_.insert(&ref);

        return ref;
    }

    /**
     * @brief Количество устройств.
     */
    std::size_t size() const noexcept { return devices_.size(); }

    /**
     * @brief Найти устройство по сетевому адресу.
     * @param addr 32-битный сетевой адрес.
     * @return Указатель на устройство или nullptr.
     *
     * В старой версии здесь был линейный поиск по vector через std::find_if.
     * В версии для работы 9 поиск выполняется через дерево хэшей.
     */
    Device* find(Device::Address addr) const noexcept {
        return addressIndex_.find(addr);
    }

    /**
     * @brief Найти все устройства с заданным адресом.
     *
     * Обычно адреса уникальны, но метод полезен для проверки обработки
     * совпадающих ключей.
     */
    std::vector<Device*> findAll(Device::Address addr) const {
        return addressIndex_.findAll(addr);
    }

    /**
     * @brief Отсортировать по убыванию приоритета, затем по имени, затем по адресу.
     *
     * Сортировка перемещает std::unique_ptr внутри vector, но сами объекты Device
     * остаются в куче по тем же адресам. Поэтому указатели в DeviceHashTree
     * остаются корректными.
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

    /**
     * @brief Доступ к основному контейнеру.
     */
    const std::vector<Ptr>& data() const noexcept { return devices_; }

    /**
     * @brief Доступ к индексу по адресу.
     */
    const DeviceHashTree& addressIndex() const noexcept { return addressIndex_; }

private:
    std::vector<Ptr> devices_;
    DeviceHashTree addressIndex_;
};