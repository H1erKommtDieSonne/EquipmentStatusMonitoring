#pragma once
#include "Device.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <optional>
#include "ServicePriority.h"

/**
* @class DeviceCollection
* @brief Контейнер владения устройствами
* Контейнер отвечает за жизненный цикл объектов устройств и
* предоставляет базовые операции управления коллекцией
*/

class DeviceCollection {
public:
    using Storage = std::vector<std::unique_ptr<Device>>;
    using Address = Device::Address;

    /**
     * @brief Добавить устройство во владение контейнера
     * @param d Указатель владения на объект @ref Device
     */

    void add(std::unique_ptr<Device> d) { items_.push_back(std::move(d)); }

    /**
     * @brief Сконструировать устройство на месте и добавить его в коллекцию
     * Упрощает добавление производных от @ref Device типов без явного
     * создания
     * @tparam T Класс, публично наследующий @ref Device
     * @tparam Args Параметры, передаваемые в конструктор T
     * @param args Аргументы конструктора
     * @return Ссылка на только что добавленный объект
     */

    template <class T, class... Args>
    T& emplace(Args&&... args) {
        static_assert(std::is_base_of_v<Device, T>, "T must derive from Device");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        items_.push_back(std::move(ptr));
        return ref;
    }

    /**
     * @brief Количество устройств в коллекции
     * @return Число элементов
     */

    std::size_t size() const noexcept { return items_.size(); }

    /**
     * @brief Проверка на пустоту
     * @return true, если коллекция пуста
     */

    bool empty() const noexcept { return items_.empty(); }

    /**
     * @brief Найти устройство по адресу
     * @param addr Адрес для поиска
     * @return Указатель на устройство
     */

    Device* findByAddress(Address addr) noexcept {
        for (auto& p : items_) if (p->address() == addr) return p.get();
        return nullptr;
    }

    /**
     * @brief Найти устройство по адресу(константная версия)
     * @param addr Адрес для поиска
     * @return Константный Указатель на устройство
     */

    const Device* findByAddress(Address addr) const noexcept {
        for (auto& p : items_) if (p->address() == addr) return p.get();
        return nullptr;
    }

    /**
     * @brief Заменить устройство по адресу или добавить, если не найдено
     * Если устройство с адресом @p addr найдено — оно заменяется на @p d.
     * Иначе @p d просто добавляется в конец.
     * @param addr Адрес заменяемого устройства
     * @param d указатель на новое устройство
     * @return true, если была именно замена; false, если выполнено добавление
     */

    bool replace(Address addr, std::unique_ptr<Device> d) {
        for (auto& p : items_) {
            if (p->address() == addr) { p = std::move(d); return true; }
        }
        items_.push_back(std::move(d));
        return false;
    }

    /**
     * @brief Удалить все устройства с заданным адресом
     * @param addr Адрес для удаления
     * @return Количество удалённых элементов.
     */

    std::size_t eraseByAddress(Address addr) {
        auto old = items_.size();
        items_.erase(std::remove_if(items_.begin(), items_.end(),
            [=](const std::unique_ptr<Device>& p) { return p->address() == addr; }),
            items_.end());
        return old - items_.size();
    }

    /**
     * @brief Обойти все элементы, вызывая переданную функцию
     * @tparam F Функтор вида void(const Device&)
     * @param f Фунция, применяемая к каждому элементу
     */

    template <class F>
    void forEach(F&& f) const {
        for (const auto& p : items_) f(*p);
    }

    /**
    * @brief Получить список неисправных устройств, отсортированный по важности
    * Критерий сортировки по убыванию приоритета обслуживания, при равном приоритете по возрастанию адреса
    * @return Вектор сырых константных указателей на элементы коллекции, жизненный цикл управляется контейнером, указывать срок жизни не нужно
    */

    std::vector<const Device*> faultySorted() const {
        std::vector<const Device*> out;
        out.reserve(items_.size());
        for (const auto& p : items_) if (p->isFaulty()) out.push_back(p.get());

        auto rank = [](ServicePriority p) {
            switch (p) {
            case ServicePriority::High: return 3;
            case ServicePriority::Low:  return 2;
            case ServicePriority::None: return 1;
            }
            return 0;
            };

        std::sort(out.begin(), out.end(),
            [&](const Device* a, const Device* b) {
                if (rank(a->priority()) != rank(b->priority()))
                    return rank(a->priority()) > rank(b->priority());
                return a->address() < b->address();
            });
        return out;
    }

    /**
     * @brief Прямой доступ к внутреннему вектору (только чтение)
     * @return Константная ссылка на хранилище
     */

    const Storage& raw() const noexcept { return items_; }

private:
    Storage items_; ///< Внутреннее хранилище владения устройствами.
};
