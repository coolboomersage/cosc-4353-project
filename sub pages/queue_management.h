#ifndef QUEUE_MANAGEMENT_H
#define QUEUE_MANAGEMENT_H

#include "../external/sqlite-amalgamation-3510200/sqlite3.h"
#include "../external/json.hpp"
#include <string>
#include <vector>

struct QueueEntry {
    int id;
    int serviceId;
    std::string name;
    std::string reason;
    int waitTime;
};

inline int getEstimatedServiceTime(sqlite3* db, int serviceId) {
    const char* sql = "SELECT estimated_service_time FROM services WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    int estimated = -1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, serviceId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        estimated = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return estimated;
}

inline std::vector<QueueEntry> getQueueByService(sqlite3* db, int serviceId) {
    std::vector<QueueEntry> entries;
    const char* sql = "SELECT id, service_id, name, reason, wait_time "
                      "FROM queue WHERE service_id = ? ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return entries;
    }

    sqlite3_bind_int(stmt, 1, serviceId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QueueEntry q;
        q.id = sqlite3_column_int(stmt, 0);
        q.serviceId = sqlite3_column_int(stmt, 1);
        q.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        q.reason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        q.waitTime = sqlite3_column_int(stmt, 4);
        entries.push_back(q);
    }

    sqlite3_finalize(stmt);
    return entries;
}

inline bool recalculateWaitTimes(sqlite3* db, int serviceId) {
    int estimated = getEstimatedServiceTime(db, serviceId);
    if (estimated <= 0) return false;

    auto queue = getQueueByService(db, serviceId);

    const char* updateSql = "UPDATE queue SET wait_time = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (size_t i = 0; i < queue.size(); i++) {
        int waitTime = static_cast<int>(i) * estimated;

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_int(stmt, 1, waitTime);
        sqlite3_bind_int(stmt, 2, queue[i].id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

inline bool addToQueue(sqlite3* db, int serviceId, const std::string& name, const std::string& reason, std::string& message) {
    if (serviceId <= 0 || name.empty() || reason.empty()) {
        message = "Invalid queue input.";
        return false;
    }

    int estimated = getEstimatedServiceTime(db, serviceId);
    if (estimated <= 0) {
        message = "Service not found.";
        return false;
    }

    auto queue = getQueueByService(db, serviceId);
    int waitTime = static_cast<int>(queue.size()) * estimated;

    const char* sql = "INSERT INTO queue (service_id, name, reason, wait_time) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare queue insert.";
        return false;
    }

    sqlite3_bind_int(stmt, 1, serviceId);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, waitTime);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (!ok) {
        message = "Failed to add to queue.";
        return false;
    }

    recalculateWaitTimes(db, serviceId);
    message = "Added to queue successfully.";
    return true;
}

inline bool removeFromQueue(sqlite3* db, int queueId, std::string& message) {
    if (queueId <= 0) {
        message = "Invalid queue ID.";
        return false;
    }

    // Find the service first so we can recalculate after delete
    const char* findSql = "SELECT service_id FROM queue WHERE id = ?;";
    sqlite3_stmt* findStmt = nullptr;
    int serviceId = -1;

    if (sqlite3_prepare_v2(db, findSql, -1, &findStmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare queue lookup.";
        return false;
    }

    sqlite3_bind_int(findStmt, 1, queueId);
    if (sqlite3_step(findStmt) == SQLITE_ROW) {
        serviceId = sqlite3_column_int(findStmt, 0);
    }
    sqlite3_finalize(findStmt);

    if (serviceId <= 0) {
        message = "Queue entry not found.";
        return false;
    }

    const char* deleteSql = "DELETE FROM queue WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, deleteSql, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare queue delete.";
        return false;
    }

    sqlite3_bind_int(stmt, 1, queueId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (!ok || changes == 0) {
        message = "No queue entry was removed.";
        return false;
    }

    recalculateWaitTimes(db, serviceId);
    message = "Queue entry removed successfully.";
    return true;
}

inline bool serveNextInQueue(sqlite3* db, int serviceId, std::string& servedName, std::string& message) {
    const char* sql = "SELECT id, name FROM queue WHERE service_id = ? ORDER BY id ASC LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare serve-next lookup.";
        return false;
    }

    sqlite3_bind_int(stmt, 1, serviceId);

    int queueId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        queueId = sqlite3_column_int(stmt, 0);
        servedName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    if (queueId <= 0) {
        message = "No one is waiting in this queue.";
        return false;
    }

    std::string removeMessage;
    if (!removeFromQueue(db, queueId, removeMessage)) {
        message = "Failed to serve next user.";
        return false;
    }

    message = "Served next user successfully.";
    return true;
}

inline nlohmann::json queueToJson(const std::vector<QueueEntry>& entries) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& q : entries) {
        arr.push_back({
            {"id", q.id},
            {"serviceId", q.serviceId},
            {"name", q.name},
            {"reason", q.reason},
            {"waitTime", q.waitTime}
        });
    }
    return arr;
}

#endif
