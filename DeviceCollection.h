#pragma once
#include "Device.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <optional>
#include "ServicePriority.h"

//

 //
class DeviceCollection {
public:
    using Storage = std::vector<std::unique_ptr<Device>>;
    using Address = Device::Address;

    //
    void add(std::unique_ptr<Device> d) { items_.push_back(std::move(d)); }

    //
    template <class T, class... Args>
    T& emplace(Args&&... args) {
        static_assert(std::is_base_of_v<Device, T>, "T must derive from Device");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        items_.push_back(std::move(ptr));
        return ref;
    }

    std::size_t size() const noexcept { return items_.size(); }

    bool empty() const noexcept { return items_.empty(); }

    //
    Device* findByAddress(Address addr) noexcept {
        for (auto& p : items_) if (p->address() == addr) return p.get();
        return nullptr;
    }

    const Device* findByAddress(Address addr) const noexcept {
        for (auto& p : items_) if (p->address() == addr) return p.get();
        return nullptr;
    }

    //
    bool replace(Address addr, std::unique_ptr<Device> d) {
        for (auto& p : items_) {
            if (p->address() == addr) { p = std::move(d); return true; }
        }
        items_.push_back(std::move(d));
        return false;
    }

    //
    std::size_t eraseByAddress(Address addr) {
        auto old = items_.size();
        items_.erase(std::remove_if(items_.begin(), items_.end(),
            [=](const std::unique_ptr<Device>& p) { return p->address() == addr; }),
            items_.end());
        return old - items_.size();
    }

    //
    template <class F>
    void forEach(F&& f) const {
        for (const auto& p : items_) f(*p);
    }

    //
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

    //
    const Storage& raw() const noexcept { return items_; }

private:
    Storage items_;
};
