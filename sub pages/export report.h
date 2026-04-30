#pragma once
 
#include "../external/sqlite-amalgamation-3510200/sqlite3.h"
#include <string>
 
// Generates a multi-sheet .xlsx report from the queue database.
// Returns true on success, false on failure.
bool exportDatabaseReport(sqlite3* db, const std::string& outputPath);
 