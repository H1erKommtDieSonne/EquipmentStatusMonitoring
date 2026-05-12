/**
 * @file benchmark_hash_tree.cpp
 * @brief Замеры производительности для работы
 *
 * Сравниваются:
 * 1) старый вариант хранения — vector + линейный поиск;
 * 2) новый вариант — дерево хэшей по сетевому адресу.
 *
 * Для честности владение устройствами всегда хранится в std::unique_ptr.
 * DeviceHashTree хранит только Device* как индекс.
 */

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Device.h"
#include "DeviceHashTree.h"
#include "HealthyDevice.h"
#include "ServicePriority.h"

namespace {

    /**
     * @brief Переменная, которая мешает компилятору полностью удалить поиск
     * при оптимизации.
     */
    volatile std::uintptr_t benchmarkSink = 0;

    /**
     * @brief Одна строка результата benchmark.
     */
    struct BenchmarkResult {
        std::size_t containerSize{};
        double vectorInsertNs{};
        double hashTreeInsertNs{};
        double vectorFindNs{};
        double hashTreeFindNs{};
    };

    /**
     * @brief Создать тестовый 32-битный сетевой адрес.
     *
     * База 3232235520 соответствует 192.168.0.0.
     */
    Device::Address makeAddress(std::uint32_t index) {
        constexpr std::uint32_t base = 3232235520u;
        return static_cast<Device::Address>(base + index);
    }

    /**
     * @brief Создать исправное устройство для тестов.
     */
    std::unique_ptr<Device> makeDevice(std::uint32_t index) {
        return std::make_unique<HealthyDevice>(
            "Device-" + std::to_string(index),
            makeAddress(index),
            ServicePriority::Low,
            static_cast<std::uint64_t>(1000 + index)
        );
    }

    /**
     * @brief Собрать vector из count устройств.
     */
    std::vector<std::unique_ptr<Device>> makeVectorStorage(std::size_t count) {
        std::vector<std::unique_ptr<Device>> devices;
        devices.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            devices.push_back(makeDevice(static_cast<std::uint32_t>(i)));
        }

        return devices;
    }

    /**
     * @brief Построить дерево хэшей по существующему vector.
     */
    DeviceHashTree buildHashTree(const std::vector<std::unique_ptr<Device>>& devices) {
        DeviceHashTree tree;

        for (const auto& device : devices) {
            tree.insert(device.get());
        }

        return tree;
    }

    /**
     * @brief Линейный поиск по vector.
     *
     * Это аналог старого поиска из DeviceCollection.
     */
    Device* linearFind(
        const std::vector<std::unique_ptr<Device>>& devices,
        Device::Address address
    ) {
        auto it = std::find_if(
            devices.begin(),
            devices.end(),
            [address](const std::unique_ptr<Device>& device) {
                return device != nullptr && device->address() == address;
            }
        );

        return it == devices.end() ? nullptr : it->get();
    }

    /**
     * @brief Измерить среднее время добавления одного элемента в vector.
     *
     * Создание объектов вынесено за пределы замеряемого участка.
     * Измеряется именно операция добавления указателя владения в контейнер.
     */
    double measureVectorInsertNs(std::size_t initialSize, std::size_t repeats) {
        auto devices = makeVectorStorage(initialSize);
        devices.reserve(initialSize + repeats);

        std::vector<std::unique_ptr<Device>> newDevices;
        newDevices.reserve(repeats);

        for (std::size_t i = 0; i < repeats; ++i) {
            newDevices.push_back(makeDevice(static_cast<std::uint32_t>(initialSize + i + 1000000)));
        }

        const auto start = std::chrono::steady_clock::now();

        for (auto& device : newDevices) {
            devices.push_back(std::move(device));
        }

        const auto finish = std::chrono::steady_clock::now();

        const auto totalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
            finish - start
        ).count();

        return static_cast<double>(totalNs) / static_cast<double>(repeats);
    }

    /**
     * @brief Измерить среднее время добавления одного элемента в дерево хэшей.
     *
     * Дерево не владеет устройствами, поэтому сами устройства заранее хранятся
     * в отдельном vector.
     */
    double measureHashTreeInsertNs(std::size_t initialSize, std::size_t repeats) {
        auto devices = makeVectorStorage(initialSize + repeats);
        DeviceHashTree tree;

        for (std::size_t i = 0; i < initialSize; ++i) {
            tree.insert(devices[i].get());
        }

        const auto start = std::chrono::steady_clock::now();

        for (std::size_t i = initialSize; i < initialSize + repeats; ++i) {
            tree.insert(devices[i].get());
        }

        const auto finish = std::chrono::steady_clock::now();

        const auto totalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
            finish - start
        ).count();

        return static_cast<double>(totalNs) / static_cast<double>(repeats);
    }

    /**
     * @brief Измерить среднее время линейного поиска по vector.
     */
    double measureVectorFindNs(std::size_t containerSize, std::size_t repeats) {
        auto devices = makeVectorStorage(containerSize);

        // Ищем последний элемент: для линейного поиска это самый тяжёлый случай.
        const Device::Address targetAddress = makeAddress(static_cast<std::uint32_t>(containerSize - 1));

        std::uintptr_t checksum = 0;

        const auto start = std::chrono::steady_clock::now();

        for (std::size_t i = 0; i < repeats; ++i) {
            Device* found = linearFind(devices, targetAddress);
            checksum += reinterpret_cast<std::uintptr_t>(found);
        }

        const auto finish = std::chrono::steady_clock::now();

        benchmarkSink = checksum;

        const auto totalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
            finish - start
        ).count();

        return static_cast<double>(totalNs) / static_cast<double>(repeats);
    }

    /**
     * @brief Измерить среднее время поиска по дереву хэшей.
     */
    double measureHashTreeFindNs(std::size_t containerSize, std::size_t repeats) {
        auto devices = makeVectorStorage(containerSize);
        DeviceHashTree tree = buildHashTree(devices);

        const Device::Address targetAddress = makeAddress(static_cast<std::uint32_t>(containerSize - 1));

        std::uintptr_t checksum = 0;

        const auto start = std::chrono::steady_clock::now();

        for (std::size_t i = 0; i < repeats; ++i) {
            Device* found = tree.find(targetAddress);
            checksum += reinterpret_cast<std::uintptr_t>(found);
        }

        const auto finish = std::chrono::steady_clock::now();

        benchmarkSink = checksum;

        const auto totalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
            finish - start
        ).count();

        return static_cast<double>(totalNs) / static_cast<double>(repeats);
    }

    /**
     * @brief Выполнить все измерения для одного размера контейнера.
     */
    BenchmarkResult runBenchmarkForSize(std::size_t size) {
        constexpr std::size_t insertRepeats = 1000;
        constexpr std::size_t findRepeats = 1000;

        BenchmarkResult result;
        result.containerSize = size;
        result.vectorInsertNs = measureVectorInsertNs(size, insertRepeats);
        result.hashTreeInsertNs = measureHashTreeInsertNs(size, insertRepeats);
        result.vectorFindNs = measureVectorFindNs(size, findRepeats);
        result.hashTreeFindNs = measureHashTreeFindNs(size, findRepeats);

        return result;
    }

    /**
     * @brief Напечатать заголовок таблицы.
     */
    void printHeader(std::ostream& out) {
        out
            << std::setw(12) << "N"
            << std::setw(20) << "vector add, ns"
            << std::setw(22) << "hash tree add, ns"
            << std::setw(21) << "vector find, ns"
            << std::setw(24) << "hash tree find, ns"
            << '\n';
    }

    /**
     * @brief Напечатать одну строку таблицы.
     */
    void printResult(std::ostream& out, const BenchmarkResult& result) {
        out
            << std::setw(12) << result.containerSize
            << std::setw(20) << std::fixed << std::setprecision(2) << result.vectorInsertNs
            << std::setw(22) << std::fixed << std::setprecision(2) << result.hashTreeInsertNs
            << std::setw(21) << std::fixed << std::setprecision(2) << result.vectorFindNs
            << std::setw(24) << std::fixed << std::setprecision(2) << result.hashTreeFindNs
            << '\n';
    }

    /**
     * @brief Записать результаты в CSV-файл.
     */
    void writeCsv(const std::vector<BenchmarkResult>& results, const std::string& filename) {
        std::ofstream file(filename);

        file << "N,vector_insert_ns,hash_tree_insert_ns,vector_find_ns,hash_tree_find_ns\n";

        for (const auto& result : results) {
            file
                << result.containerSize << ','
                << result.vectorInsertNs << ','
                << result.hashTreeInsertNs << ','
                << result.vectorFindNs << ','
                << result.hashTreeFindNs << '\n';
        }
    }

} // namespace

int main() {
    const std::vector<std::size_t> sizes = {
        10,
        100,
        1000,
        10000,
        100000
    };

    std::vector<BenchmarkResult> results;
    results.reserve(sizes.size());

    std::cout << "Работа 9: DeviceHashTree benchmark\n";
    std::cout << "Address key: 32-bit network address\n\n";

    printHeader(std::cout);

    for (const std::size_t size : sizes) {
        BenchmarkResult result = runBenchmarkForSize(size);
        results.push_back(result);
        printResult(std::cout, result);
    }

    writeCsv(results, "benchmark_results.csv");

    std::cout << "\nРезультаты в файле benchmark_results.csv\n";

    return 0;
}