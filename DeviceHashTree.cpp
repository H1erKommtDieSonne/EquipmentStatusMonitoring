/**
 * @file DeviceHashTree.cpp
 * @brief Реализация дерева хэшей по сетевому адресу устройства.
 */

#include "DeviceHashTree.h"

#include <algorithm>

std::uint8_t DeviceHashTree::byteAt(Device::Address address, std::size_t level) noexcept {
    // Адрес состоит из четырёх байтов.
    // Берём байты от старшего к младшему.
    const std::size_t shift = 8 * (kAddressBytes - 1 - level);
    return static_cast<std::uint8_t>((address >> shift) & 0xFFu);
}

void DeviceHashTree::insert(Device* device) {
    if (device == nullptr) {
        return;
    }

    Node* current = &root_;
    const Device::Address address = device->address();

    // Проходим четыре уровня дерева — по одному уровню на каждый байт адреса.
    for (std::size_t level = 0; level < kAddressBytes; ++level) {
        const std::uint8_t index = byteAt(address, level);

        if (!current->children[index]) {
            current->children[index] = std::make_unique<Node>();
        }

        current = current->children[index].get();
    }

    // В листе допускается список устройств.
    // Это нужно на случай совпадения адресов или коллизий.
    auto alreadyExists = std::find(current->devices.begin(), current->devices.end(), device);
    if (alreadyExists == current->devices.end()) {
        current->devices.push_back(device);
        ++size_;
    }
}

Device* DeviceHashTree::find(Device::Address address) const noexcept {
    const Node* leaf = findLeaf(address);

    if (leaf == nullptr) {
        return nullptr;
    }

    auto it = std::find_if(
        leaf->devices.begin(),
        leaf->devices.end(),
        [address](const Device* device) {
            return device != nullptr && device->address() == address;
        }
    );

    return it == leaf->devices.end() ? nullptr : *it;
}

std::vector<Device*> DeviceHashTree::findAll(Device::Address address) const {
    std::vector<Device*> result;

    const Node* leaf = findLeaf(address);
    if (leaf == nullptr) {
        return result;
    }

    for (Device* device : leaf->devices) {
        if (device != nullptr && device->address() == address) {
            result.push_back(device);
        }
    }

    return result;
}

bool DeviceHashTree::remove(Device::Address address) {
    Node* leaf = findLeaf(address);

    if (leaf == nullptr) {
        return false;
    }

    const std::size_t oldSize = leaf->devices.size();

    leaf->devices.erase(
        std::remove_if(
            leaf->devices.begin(),
            leaf->devices.end(),
            [address](const Device* device) {
                return device != nullptr && device->address() == address;
            }
        ),
        leaf->devices.end()
    );

    const std::size_t removedCount = oldSize - leaf->devices.size();
    size_ -= removedCount;

    return removedCount > 0;
}

bool DeviceHashTree::remove(Device* device) {
    if (device == nullptr) {
        return false;
    }

    Node* leaf = findLeaf(device->address());

    if (leaf == nullptr) {
        return false;
    }

    const std::size_t oldSize = leaf->devices.size();

    leaf->devices.erase(
        std::remove(leaf->devices.begin(), leaf->devices.end(), device),
        leaf->devices.end()
    );

    const std::size_t removedCount = oldSize - leaf->devices.size();
    size_ -= removedCount;

    return removedCount > 0;
}

void DeviceHashTree::clear() noexcept {
    root_ = Node{};
    size_ = 0;
}

DeviceHashTree::Node* DeviceHashTree::findLeaf(Device::Address address) noexcept {
    Node* current = &root_;

    for (std::size_t level = 0; level < kAddressBytes; ++level) {
        const std::uint8_t index = byteAt(address, level);

        if (!current->children[index]) {
            return nullptr;
        }

        current = current->children[index].get();
    }

    return current;
}

const DeviceHashTree::Node* DeviceHashTree::findLeaf(Device::Address address) const noexcept {
    const Node* current = &root_;

    for (std::size_t level = 0; level < kAddressBytes; ++level) {
        const std::uint8_t index = byteAt(address, level);

        if (!current->children[index]) {
            return nullptr;
        }

        current = current->children[index].get();
    }

    return current;
}