#ifndef databaseManager_h
#define databaseManager_h

#include "../external/sqlite-amalgamation-3510200/sqlite3.h"
#include "../external/phc-winner-argon2-20190702/include/argon2.h"
#include "account.h"
#include <cstring>
#include <random>
#include <iomanip>
#include <iostream>

#define DATABASE_FILE_LOCATION getExecutableDirectory() + "/QueuesmartDatabase.db"

std::string hashPassword(const std::string& password) {
    const uint32_t HASH_LEN = 32;
    const uint32_t SALT_LEN = 16;
    const uint32_t T_COST = 2;
    const uint32_t M_COST = 65536;
    const uint32_t PARALLELISM = 1;
    
    // Generate random salt
    uint8_t salt[SALT_LEN];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < SALT_LEN; i++) {
        salt[i] = dis(gen);
    }
    
    uint8_t hash[HASH_LEN];
    int result = argon2id_hash_raw(
        T_COST,
        M_COST,
        PARALLELISM,
        password.c_str(),
        password.length(),
        salt,
        SALT_LEN,
        hash,
        HASH_LEN
    );
    
    if (result != ARGON2_OK) {
        std::cerr << "Argon2 hashing failed: " << argon2_error_message(result) << std::endl;
        return "";
    }
    
    std::stringstream ss;
    for (int i = 0; i < SALT_LEN; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    ss << ":";
    for (int i = 0; i < HASH_LEN; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

bool initDatabase(sqlite3* db) {
    char* errMsg = nullptr;

    // Create accounts table
    const char* createAccountsSQL =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "hash TEXT NOT NULL, "
        "email TEXT NOT NULL UNIQUE, "
        "auth INTEGER NOT NULL);";

    int rc = sqlite3_exec(db, createAccountsSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to create accounts table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Create services table
    const char* createServicesSQL =
        "CREATE TABLE IF NOT EXISTS services ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT UNIQUE NOT NULL, "
        "description TEXT NOT NULL, "
        "estimated_service_time INTEGER NOT NULL, "
        "length INTEGER NOT NULL, "
        "priority INTEGER NOT NULL, "
        "created_date TEXT NOT NULL);";

    rc = sqlite3_exec(db, createServicesSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to create services table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Create queue table
    const char* createQueueSQL =
        "CREATE TABLE IF NOT EXISTS queue ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "service_id INTEGER NOT NULL, "
        "position INTEGER NOT NULL, "
        "name TEXT NOT NULL, "
        "reason TEXT NOT NULL, "
        "wait_time INTEGER NOT NULL, "
        "status TEXT NOT NULL DEFAULT 'open', "
        "created_date TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (service_id) REFERENCES services(id));";
    

    rc = sqlite3_exec(db, createQueueSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to create queue table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Insert default root account
    std::string defaultPasswordHash = hashPassword("root123");
    std::string insertDefaultSQL =
        "INSERT INTO accounts (username, hash, email, auth) VALUES ('root', '"
        + defaultPasswordHash + "', 'root@root', 3);";

    rc = sqlite3_exec(db, insertDefaultSQL.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to insert default account: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Seed services table
    const char* insertServicesSQL =
        "INSERT OR IGNORE INTO services (name, description, estimated_service_time, length, priority, created_date) VALUES "
        "('Advising', 'Academic advising support', 20, 0, 2, DATE('now')), "
        "('Tutoring', 'Tutoring assistance', 30, 0, 2, DATE('now')), "
        "('Tech Support', 'Technical support services', 15, 0, 0, DATE('now'));";

    
    rc = sqlite3_exec(db, insertServicesSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to insert services: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Trigger: increment length when someone joins the queue
    const char* triggerInsertSQL =
        "CREATE TRIGGER IF NOT EXISTS update_length_on_insert "
        "AFTER INSERT ON queue "
        "BEGIN "
        "    UPDATE services "
        "    SET length = (SELECT COUNT(*) FROM queue WHERE service_id = NEW.service_id) "
        "    WHERE id = NEW.service_id; "
        "END;";

    // Trigger: decrement length when someone leaves the queue
    const char* triggerDeleteSQL =
        "CREATE TRIGGER IF NOT EXISTS update_length_on_delete "
        "AFTER DELETE ON queue "
        "BEGIN "
        "    UPDATE services "
        "    SET length = (SELECT COUNT(*) FROM queue WHERE service_id = OLD.service_id) "
        "    WHERE id = OLD.service_id; "
        "END;";

    sqlite3_exec(db, triggerInsertSQL, nullptr, nullptr, nullptr);
    sqlite3_exec(db, triggerDeleteSQL, nullptr, nullptr, nullptr);

    // Seed queue table — relies on services being inserted with IDs 1, 2, 3
    const char* insertQueueSQL =
        "INSERT INTO queue (service_id, position, name, reason, wait_time) VALUES "
        // Advising (service_id = 1)
        "(1, 1, 'Alice Johnson',  'Course registration help', 2),  "
        "(1, 2, 'Bob Smith',      'Degree audit question',    7),  "
        "(1, 3, 'Carol Williams', 'Transfer credit inquiry',  13), "
        "(1, 4, 'David Lee',      'Scholarship advising',     18), "
        "(1, 5, 'Eva Martinez',   'General advising',         24), "
        // Tutoring (service_id = 2)
        "(2, 1, 'Frank Brown',   'Calculus II help',      3),  "
        "(2, 2, 'Grace Kim',     'Python debugging',      9),  "
        "(2, 3, 'Henry Davis',   'Linear algebra review', 15), "
        "(2, 4, 'Isla Thompson', 'Essay feedback',        20), "
        // Tech Support (service_id = 3)
        "(3, 1, 'Jack Wilson', 'VPN setup issue',    5),  "
        "(3, 2, 'Karen Moore', 'Printer not working', 11);";

    rc = sqlite3_exec(db, insertQueueSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to insert queue entries: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Create history table
    const char* createHistorySQL =
    "CREATE TABLE IF NOT EXISTS history ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "user_id INTEGER NOT NULL, "
    "message TEXT NOT NULL, "
    "timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
    "status TEXT NOT NULL DEFAULT 'sent' CHECK(status IN ('sent', 'viewed')), "
    "queue_id INTEGER, "
    "FOREIGN KEY (user_id) REFERENCES accounts(id), "
    "FOREIGN KEY (queue_id) REFERENCES queue(id));";

    rc = sqlite3_exec(db, createHistorySQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to create history table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    std::cout << "Default database created\nroot level login info\nusername: root\npassword: root123\n" << std::endl;
    return true;
}

#endif
