#pragma once
/**
 * @file WorkerRepository.h
 * @brief CRUD для таблицы workers (минимум, нужный для тестов).
 */

#include "SqliteStorage.h"


#include <string>
#include <vector>
#include <cstdint>

 /// @brief Модель работника (в рамках тестов).
struct WorkerRow {
    std::int64_t id{};
    std::string  name;
    int          max_jobs{};
    std::string  skill;
};

/**
 * @class WorkerRepository
 * @brief Добавление и выборка работников.
 */
class WorkerRepository {
public:
    explicit WorkerRepository(SqliteStorage& s) : st_(s) {}

    /// @brief Вставить работника и вернуть его id.
    std::int64_t insert(const std::string& name, int max_jobs, const std::string& skill);

    /// @brief Получить всех работников.
    std::vector<WorkerRow> list() const;

private:
    SqliteStorage& st_;
};
