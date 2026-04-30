#include "export report.h"

#include "../external/libxlsxwriter-1.2.4/libxlsxwriter-1.2.4/include/xlsxwriter.h"
#include "../external/sqlite-amalgamation-3510200/sqlite3.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

struct Styles {
    lxw_format* title       = nullptr;  // large bold title
    lxw_format* sectionHdr  = nullptr;  // section label (bold, blue bg)
    lxw_format* colHdr      = nullptr;  // column header (bold, grey bg)
    lxw_format* bold        = nullptr;
    lxw_format* normal      = nullptr;
    lxw_format* number      = nullptr;  // right-aligned integer
    lxw_format* decimal     = nullptr;  // 1 decimal place
    lxw_format* center      = nullptr;
};

static Styles createStyles(lxw_workbook* wb) {
    Styles s;

    s.title = workbook_add_format(wb);
    format_set_bold(s.title);
    format_set_font_size(s.title, 16);
    format_set_font_color(s.title, 0x1F4788);

    s.sectionHdr = workbook_add_format(wb);
    format_set_bold(s.sectionHdr);
    format_set_font_color(s.sectionHdr, 0xFFFFFF);
    format_set_bg_color(s.sectionHdr, 0x1F4788);
    format_set_align(s.sectionHdr, LXW_ALIGN_CENTER);

    s.colHdr = workbook_add_format(wb);
    format_set_bold(s.colHdr);
    format_set_bg_color(s.colHdr, 0xD9E1F2);
    format_set_border(s.colHdr, LXW_BORDER_THIN);

    s.bold = workbook_add_format(wb);
    format_set_bold(s.bold);

    s.normal = workbook_add_format(wb);

    s.number = workbook_add_format(wb);
    format_set_align(s.number, LXW_ALIGN_RIGHT);

    s.decimal = workbook_add_format(wb);
    format_set_num_format(s.decimal, "0.0");
    format_set_align(s.decimal, LXW_ALIGN_RIGHT);

    s.center = workbook_add_format(wb);
    format_set_align(s.center, LXW_ALIGN_CENTER);

    return s;
}

static int writeQueryTable(lxw_worksheet*    ws,
                           sqlite3*          db,
                           const char*       sql,
                           lxw_row_t         startRow,
                           lxw_col_t         startCol,
                           const Styles&     st)
{
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << "\n  Query: " << sql << "\n";
        return 0;
    }

    int colCount = sqlite3_column_count(stmt);

    // Write column headers
    for (int c = 0; c < colCount; ++c) {
        worksheet_write_string(ws,
                               startRow,
                               static_cast<lxw_col_t>(startCol + c),
                               sqlite3_column_name(stmt, c),
                               st.colHdr);
    }

    lxw_row_t row = startRow + 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int c = 0; c < colCount; ++c) {
            lxw_col_t col = static_cast<lxw_col_t>(startCol + c);
            int type = sqlite3_column_type(stmt, c);

            if (type == SQLITE_NULL) {
                worksheet_write_string(ws, row, col, "", st.normal);
            } else if (type == SQLITE_INTEGER) {
                worksheet_write_number(ws, row, col,
                                       sqlite3_column_int64(stmt, c), st.number);
            } else if (type == SQLITE_FLOAT) {
                worksheet_write_number(ws, row, col,
                                       sqlite3_column_double(stmt, c), st.decimal);
            } else {
                const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, c));
                worksheet_write_string(ws, row, col, text ? text : "", st.normal);
            }
        }
        ++row;
    }

    sqlite3_finalize(stmt);
    return static_cast<int>(row - startRow - 1); // data rows written
}

static void writeSectionBanner(lxw_worksheet* ws,
                               lxw_row_t      row,
                               lxw_col_t      startCol,
                               lxw_col_t      endCol,
                               const char*    label,
                               const Styles&  st)
{
    worksheet_merge_range(ws, row, startCol, row, endCol, label, st.sectionHdr);
}

static double queryScalar(sqlite3* db, const char* sql, double fallback = 0.0) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return fallback;
    double result = fallback;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return result;
}

static void writeSummarySheet(lxw_worksheet* ws, sqlite3* db, const Styles& st) {

    // Set column widths for readability
    worksheet_set_column(ws, 0, 0, 22, nullptr);
    worksheet_set_column(ws, 1, 1, 28, nullptr);
    worksheet_set_column(ws, 2, 2, 20, nullptr);
    worksheet_set_column(ws, 3, 3, 18, nullptr);
    worksheet_set_column(ws, 4, 4, 16, nullptr);
    worksheet_set_column(ws, 5, 5, 16, nullptr);
    worksheet_set_column(ws, 6, 6, 16, nullptr);

    lxw_row_t row = 0;

    // ── Title ────────────────────────────────────────────────────────────────
    worksheet_merge_range(ws, row, 0, row, 6, "Queue Management System — Report", st.title);
    row += 2;
    writeSectionBanner(ws, row, 0, 6, "QUEUE USAGE STATISTICS", st);
    row++;

    // KPI labels / formulas computed from DB
    double totalEnqueued  = queryScalar(db, "SELECT COUNT(*) FROM queue;");
    double openEntries    = queryScalar(db, "SELECT COUNT(*) FROM queue WHERE status='open';");
    double avgWait        = queryScalar(db, "SELECT AVG(wait_time) FROM queue;");
    double maxWait        = queryScalar(db, "SELECT MAX(wait_time) FROM queue;");
    double totalServices  = queryScalar(db, "SELECT COUNT(*) FROM services;");
    double totalAccounts  = queryScalar(db, "SELECT COUNT(*) FROM accounts;");

    struct KPI { const char* label; double value; bool isDecimal; };
    KPI kpis[] = {
        { "Total Queue Entries",      totalEnqueued, false },
        { "Currently Open Entries",   openEntries,   false },
        { "Average Wait Time (min)",  avgWait,       true  },
        { "Max Wait Time (min)",      maxWait,       false },
        { "Total Services",           totalServices, false },
        { "Total Accounts",           totalAccounts, false },
    };

    worksheet_write_string(ws, row, 0, "Metric",       st.colHdr);
    worksheet_write_string(ws, row, 1, "Value",        st.colHdr);
    row++;

    for (auto& k : kpis) {
        worksheet_write_string(ws,  row, 0, k.label, st.bold);
        worksheet_write_number(ws, row, 1, k.value,  k.isDecimal ? st.decimal : st.number);
        row++;
    }
    row++;

    writeSectionBanner(ws, row, 0, 6, "SERVICE DETAILS & QUEUE ACTIVITY", st);
    row++;

    const char* serviceActivitySQL =
        "SELECT "
        "  s.name          AS service_name, "
        "  s.description   AS description, "
        "  s.estimated_service_time AS est_service_time_min, "
        "  s.priority      AS priority, "
        "  s.length        AS queue_length, "
        "  COUNT(q.id)     AS total_entries, "
        "  ROUND(AVG(q.wait_time), 1) AS avg_wait_min, "
        "  MAX(q.wait_time) AS max_wait_min, "
        "  s.created_date  AS created_date "
        "FROM services s "
        "LEFT JOIN queue q ON q.service_id = s.id "
        "GROUP BY s.id "
        "ORDER BY s.priority DESC, s.name;";

    writeQueryTable(ws, db, serviceActivitySQL, row, 0, st);
    // advance row past the results (need to count)
    double svcRows = queryScalar(db, "SELECT COUNT(*) FROM services;");
    row += static_cast<lxw_row_t>(svcRows) + 2; // header + data + gap

    writeSectionBanner(ws, row, 0, 6, "USER / CUSTOMER QUEUE PARTICIPATION HISTORY", st);
    row++;

    const char* participationSQL =
        "SELECT "
        "  q.name            AS customer_name, "
        "  s.name            AS service, "
        "  q.reason          AS reason, "
        "  q.position        AS queue_position, "
        "  q.wait_time       AS wait_time_min, "
        "  q.status          AS status, "
        "  q.created_date    AS date_joined "
        "FROM queue q "
        "JOIN services s ON s.id = q.service_id "
        "ORDER BY s.name, q.position;";

    writeQueryTable(ws, db, participationSQL, row, 0, st);
}


static void writeRawTableSheet(lxw_worksheet* ws,
                               sqlite3*       db,
                               const char*    tableName,
                               const Styles&  st)
{
    // Auto-size columns generously
    worksheet_set_column(ws, 0, 15, 20, nullptr);

    lxw_row_t row = 0;

    // Banner
    char title[128];
    snprintf(title, sizeof(title), "RAW TABLE: %s", tableName);
    worksheet_merge_range(ws, row, 0, row, 7, title, st.sectionHdr);
    row++;

    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s;", tableName);
    writeQueryTable(ws, db, sql, row, 0, st);
}

bool exportDatabaseReport(sqlite3* db, const std::string& outputPath) {
    if (!db) {
        std::cerr << "exportDatabaseReport: null db pointer\n";
        return false;
    }

    lxw_workbook_options options = {};
    options.constant_memory = 0;

    lxw_workbook* wb = workbook_new_opt(outputPath.c_str(), &options);
    if (!wb) {
        std::cerr << "exportDatabaseReport: failed to create workbook at " << outputPath << "\n";
        return false;
    }

    Styles st = createStyles(wb);
    
    lxw_worksheet* wsSummary = workbook_add_worksheet(wb, "Summary");
    writeSummarySheet(wsSummary, db, st);

    const char* tableListSQL =
        "SELECT name FROM sqlite_master "
        "WHERE type='table' AND name NOT LIKE 'sqlite_%' "
        "ORDER BY name;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, tableListSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* tbl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (!tbl) continue;

            lxw_worksheet* ws = workbook_add_worksheet(wb, tbl);
            writeRawTableSheet(ws, db, tbl, st);
        }
        sqlite3_finalize(stmt);
    }

    lxw_error err = workbook_close(wb);
    if (err != LXW_NO_ERROR) {
        std::cerr << "exportDatabaseReport: workbook_close error: "
                  << lxw_strerror(err) << "\n";
        return false;
    }

    std::cout << "Report exported to: " << outputPath << "\n";
    return true;
}