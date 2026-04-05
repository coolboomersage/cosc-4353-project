#ifndef SERVICE_MANAGEMENT_H
#define SERVICE_MANAGEMENT_H

#include "../external/sqlite-amalgamation-3510200/sqlite3.h"
#include "../external/json.hpp"
#include <string>
#include <vector>
#include <iostream>

struct Service {
    int id;
    std::string name;
    std::string description;
    int estimatedServiceTime;
    int priority;
    std::string createdDate;
};

inline bool addService(sqlite3* db, const std::string& name, int estimatedServiceTime, std::string& message) {
    if (name.empty() || estimatedServiceTime <= 0) {
        message = "Invalid service name or estimated time.";
        return false;
    }

    const char* sql = "INSERT INTO services (name, estimated_service_time) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare insert statement.";
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, estimatedServiceTime);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (!ok) {
        message = "Failed to add service. It may already exist.";
        return false;
    }

    message = "Service added successfully.";
    return true;
}

inline std::vector<Service> getAllServices(sqlite3* db) {
    std::vector<Service> services;
    const char* sql = "SELECT id, name, description, estimated_service_time, priority, created_date FROM services ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return services;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Service s;
        s.id = sqlite3_column_int(stmt, 0);
        s.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        s.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        s.estimatedServiceTime = sqlite3_column_int(stmt, 3);
        s.priority = sqlite3_column_int(stmt, 4);
        s.createdDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        services.push_back(s);
    }

    sqlite3_finalize(stmt);
    return services;
}

inline bool updateService(sqlite3* db, int serviceId, const std::string& name, int estimatedServiceTime, std::string& message) {
    if (serviceId <= 0 || name.empty() || estimatedServiceTime <= 0) {
        message = "Invalid service update input.";
        return false;
    }

    const char* sql = "UPDATE services SET name = ?, estimated_service_time = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare update statement.";
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, estimatedServiceTime);
    sqlite3_bind_int(stmt, 3, serviceId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (!ok || changes == 0) {
        message = "No service was updated.";
        return false;
    }

    message = "Service updated successfully.";
    return true;
}

inline bool deleteService(sqlite3* db, int serviceId, std::string& message) {
    if (serviceId <= 0) {
        message = "Invalid service ID.";
        return false;
    }

    // Prevent deletion if queue entries still reference this service
    const char* checkSql = "SELECT COUNT(*) FROM queue WHERE service_id = ?;";
    sqlite3_stmt* checkStmt = nullptr;

    if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare dependency check.";
        return false;
    }

    sqlite3_bind_int(checkStmt, 1, serviceId);

    int count = 0;
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        count = sqlite3_column_int(checkStmt, 0);
    }
    sqlite3_finalize(checkStmt);

    if (count > 0) {
        message = "Cannot delete service with active queue entries.";
        return false;
    }

    const char* deleteSql = "DELETE FROM services WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, deleteSql, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare delete statement.";
        return false;
    }

    sqlite3_bind_int(stmt, 1, serviceId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (!ok || changes == 0) {
        message = "No service was deleted.";
        return false;
    }

    message = "Service deleted successfully.";
    return true;
}

inline nlohmann::json servicesToJson(const std::vector<Service>& services) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& s : services) {
        arr.push_back({
            {"id", s.id},
            {"name", s.name},
            {"description", s.description},
            {"estimatedServiceTime", s.estimatedServiceTime},
            {"priority", s.priority},
            {"createdDate", s.createdDate}
        });
    }
    return arr;
}


#endif
