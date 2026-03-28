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
    int position;
    int waitTime;
};

struct ServiceEntry{
    int id;
    std::string name;
    int estServiceTime;
    int length;
};

inline std::vector<ServiceEntry> getServices(sqlite3* db){
    std::vector<ServiceEntry> services;
    
    const char* query = "SELECT id, name, estimated_service_time, length FROM services;";
    sqlite3_stmt* stmt = nullptr;
    
    if(sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK){
        return services;
    }
    
    while(sqlite3_step(stmt) == SQLITE_ROW){
        ServiceEntry entry;

        entry.id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        entry.name = name ? reinterpret_cast<const char*>(name) : "";
        entry.estServiceTime = sqlite3_column_int(stmt, 2);
        entry.length = sqlite3_column_int(stmt, 3);
        
        services.push_back(entry);
    }
    
    sqlite3_finalize(stmt);
    return services;
}

inline int getServiceLengthById(sqlite3* db, int id) {
    if (!db) return -1;

    const char* sql = "SELECT length FROM services WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int length = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        length = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return length;
}

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
    const char* sql = "SELECT id, service_id, position, name, reason, wait_time "
                      "FROM queue WHERE service_id = ? ORDER BY position ASC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return entries;
    }

    sqlite3_bind_int(stmt, 1, serviceId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QueueEntry q;
        q.id         = sqlite3_column_int(stmt, 0);
        q.serviceId  = sqlite3_column_int(stmt, 1);
        q.position   = sqlite3_column_int(stmt, 2);
        q.name       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        q.reason     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        q.waitTime   = sqlite3_column_int(stmt, 5);
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

    sqlite3_stmt* stmt;
    const char* insertQueueSQL =
        "INSERT INTO queue (service_id, position, name, reason, wait_time) VALUES (?, ?, ?, ?, 0);";

    sqlite3_prepare_v2(db, insertQueueSQL, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, serviceId);
    sqlite3_bind_int(stmt, 2, getServiceLengthById(db, serviceId));
    sqlite3_bind_text(stmt, 3, getUsernameById(currentUserId).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, reason.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, insertQueueSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to insert queue entries: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    recalculateWaitTimes(db, serviceId);
    sqlite3_exec(db, "PRAGMA wal_checkpoint(FULL);", nullptr, nullptr, nullptr);
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

inline int estimateWaitTimeForUser(sqlite3* db, int serviceId, int queueId) {
    int estimatedServiceTime = getEstimatedServiceTime(db, serviceId);
    if (estimatedServiceTime <= 0) {
        return -1;
    }

    auto queue = getQueueByService(db, serviceId);

    for (size_t i = 0; i < queue.size(); i++) {
        if (queue[i].id == queueId) {
            return queue[i].position * estimatedServiceTime;
        }
    }

    return -1;
}

#endif
