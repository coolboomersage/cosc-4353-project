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
    std::string status;
    std::string createdDate;
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

inline std::vector<int> getServiceIdUserCurrentlyIn(sqlite3* db, std::string name) {
    std::vector<int> serviceIds;
    
    const char* sql = "SELECT SERVICE_ID FROM queue WHERE NAME = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return serviceIds;
    }
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int serviceId = sqlite3_column_int(stmt, 0);
        serviceIds.push_back(serviceId);
    }
    
    sqlite3_finalize(stmt);
    return serviceIds;
}

inline std::string getUserReasonByID(sqlite3* db, std::string name, int serviceID) {
    std::string reason = "";
    const char* sql = "SELECT REASON FROM queue WHERE NAME = ? AND SERVICE_ID = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return reason;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, serviceID);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) {
            reason = reinterpret_cast<const char*>(text);
        }
    }

    sqlite3_finalize(stmt);
    return reason;
}

inline void removeUserFromQueue(sqlite3* db, std::string name, int serviceID) {
    // Fetch the queue ID before deleting so we can log it
    int queueId = -1;
    const char* selectSQL = "SELECT id FROM queue WHERE name = ? AND service_id = ?;";
    sqlite3_stmt* selectStmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &selectStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(selectStmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(selectStmt, 2, serviceID);
        if (sqlite3_step(selectStmt) == SQLITE_ROW) {
            queueId = sqlite3_column_int(selectStmt, 0);
        }
        sqlite3_finalize(selectStmt);
    }

    // Delete from queue
    const char* deleteSQL = "DELETE FROM queue WHERE name = ? AND service_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, serviceID);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Log to history
    if (queueId != -1) {
        const char* historySQL =
    "INSERT INTO history (user_id, message, queue_id, status) VALUES (?, ?, ?, 'sent');";
sqlite3_stmt* histStmt;

if (sqlite3_prepare_v2(db, historySQL, -1, &histStmt, nullptr) == SQLITE_OK) {
    std::string historyMessage = "User left the queue";
    sqlite3_bind_int(histStmt, 1, currentUserId);
    sqlite3_bind_text(histStmt, 2, historyMessage.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(histStmt, 3, queueId);
    sqlite3_step(histStmt);
    sqlite3_finalize(histStmt);
}
    }
    std::cout << "USER LEAVE QUEUE: " + name + "\n";
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
    const char* sql = "SELECT id, service_id, position, name, reason, wait_time, status, created_date "
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
        q.status      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        q.createdDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
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
    
    int position = static_cast<int>(queue.size()) + 1;
    int waitTime = (position - 1) * estimated;


    const char* insertQueueSQL =
        "INSERT INTO queue (service_id, position, name, reason, wait_time, status, created_date) "
        "VALUES (?, ?, ?, ?, ?, 'open', CURRENT_TIMESTAMP);";


    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, insertQueueSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        message = "Failed to prepare queue insert.";
        return false;
    }

    sqlite3_bind_int(stmt, 1, serviceId);
    sqlite3_bind_int(stmt, 2, position);
    sqlite3_bind_text(stmt, 3, getUsernameById(currentUserId).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, waitTime);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        message = "Failed to insert into queue.";
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    // Capture the new queue row's ID for the history log
    int newQueueId = static_cast<int>(sqlite3_last_insert_rowid(db));

    // Log to history
    const char* historySQL =
    "INSERT INTO history (user_id, message, queue_id, status) VALUES (?, ?, ?, 'sent');";
    sqlite3_stmt* histStmt;

    if (sqlite3_prepare_v2(db, historySQL, -1, &histStmt, nullptr) == SQLITE_OK) {
        std::string historyMessage = "User joined the queue";
        sqlite3_bind_int(histStmt, 1, currentUserId);
        sqlite3_bind_text(histStmt, 2, historyMessage.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(histStmt, 3, newQueueId);
        sqlite3_step(histStmt);
        sqlite3_finalize(histStmt);
    }

    recalculateWaitTimes(db, serviceId);
    sqlite3_exec(db, "PRAGMA wal_checkpoint(FULL);", nullptr, nullptr, nullptr);
    message = "Added to queue successfully.";
    std::cout << "USER JOIN QUEUE: " + name + " for " + reason + "\n";
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
    const char* sql = "SELECT id, name FROM queue WHERE service_id = ? ORDER BY position ASC LIMIT 1;";
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
            {"position", q.position},
            {"waitTime", q.waitTime},
            {"status", q.status},
            {"createdDate", q.createdDate}
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
            return (queue[i].position - 1) * estimatedServiceTime;
        }
    }

    return -1;
}

#endif
