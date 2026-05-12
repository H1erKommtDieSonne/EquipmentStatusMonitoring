#pragma once
/**
 * @file DeviceHashTree.h
 * @brief Дерево хэшей для быстрого поиска устройств по сетевому адресу.
 *
 * Ключом является 32-битный сетевой адрес устройства.
 *
 * Дерево строится по байтам адреса:
 * address = 192.168.1.1
 * путь: 192 -> 168 -> 1 -> 1
 *
 * В листе хранится список указателей Device*, потому что:
 * 1) одинаковые адреса теоретически могут встретиться;
 * 2) дерево не владеет устройствами, а только индексирует их.
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "Device.h"

 /**
  * @class DeviceHashTree
  * @brief Индекс устройств по 32-битному сетевому адресу.
  *
  * Важно: DeviceHashTree не владеет объектами Device.
  * Владение остаётся у DeviceCollection через std::unique_ptr<Device>.
  * Дерево хранит только обычные указатели Device*.
  */
class DeviceHashTree {
public:
    /**
     * @brief Конструктор пустого дерева.
     */
    DeviceHashTree() = default;

    /**
     * @brief Добавить устройство в дерево.
     * @param device Указатель на устройство.
     *
     * Если указатель равен nullptr, функция ничего не делает.
     * Если тот же самый указатель уже есть в листе, повторно он не добавляется.
     */
    void insert(Device* device);

    /**
     * @brief Найти первое устройство с заданным адресом.
     * @param address 32-битный сетевой адрес.
     * @return Указатель на устройство или nullptr, если устройство не найдено.
     */
    Device* find(Device::Address address) const noexcept;

    /**
     * @brief Найти все устройства с заданным адресом.
     * @param address 32-битный сетевой адрес.
     * @return Список указателей на подходящие устройства.
     *
     * В нормальной ситуации адреса уникальны, но список нужен для обработки
     * совпадений ключей.
     */
    std::vector<Device*> findAll(Device::Address address) const;

    /**
     * @brief Удалить из дерева все устройства с заданным адресом.
     * @param address 32-битный сетевой адрес.
     * @return true, если был удалён хотя бы один указатель.
     */
    bool remove(Device::Address address);

    /**
     * @brief Удалить из дерева конкретный указатель на устройство.
     * @param device Указатель на устройство.
     * @return true, если указатель был найден и удалён.
     */
    bool remove(Device* device);

    /**
     * @brief Очистить дерево.
     */
    void clear() noexcept;

    /**
     * @brief Проверить, пустое ли дерево.
     */
    bool empty() const noexcept { return size_ == 0; }

    /**
     * @brief Количество указателей на устройства, сохранённых в дереве.
     */
    std::size_t size() const noexcept { return size_; }

private:
    static constexpr std::size_t kAddressBytes = 4;
    static constexpr std::size_t kBranchCount = 256;

    /**
     * @struct Node
     * @brief Узел дерева.
     *
     * Каждый узел имеет до 256 потомков, по одному на каждое значение байта.
     * Вектор devices заполняется только на глубине 4, то есть после обработки
     * всех четырёх байтов адреса.
     */
    struct Node {
        std::array<std::unique_ptr<Node>, kBranchCount> children{};
        std::vector<Device*> devices{};
    };

    /**
     * @brief Получить байт адреса по номеру уровня.
     * @param address 32-битный адрес.
     * @param level Номер уровня от 0 до 3.
     * @return Значение байта от 0 до 255.
     *
     * level = 0 соответствует старшему байту.
     * Например, для 192.168.1.1:
     * level 0 -> 192
     * level 1 -> 168
     * level 2 -> 1
     * level 3 -> 1
     */
    static std::uint8_t byteAt(Device::Address address, std::size_t level) noexcept;

    /**
     * @brief Найти лист дерева для адреса.
     * @return Указатель на узел-лист или nullptr.
     */
    Node* findLeaf(Device::Address address) noexcept;

    /**
     * @brief Найти лист дерева для адреса. Константная версия.
     * @return Указатель на узел-лист или nullptr.
     */
    const Node* findLeaf(Device::Address address) const noexcept;

    Node root_{};
    std::size_t size_{ 0 };
};