// courses_db.cpp
// Compile: g++ courses_db.cpp -o courses_db -lsqlite3

#include <iostream>
#include <string>
#include <sqlite3.h>

// ── Callback: prints each row from a query ──────────────────────────────────
static int printRowCallback(void* /*unused*/, int colCount,
                             char** colValues, char** colNames) {
    for (int i = 0; i < colCount; ++i) {
        std::cout << colNames[i] << ": "
                  << (colValues[i] ? colValues[i] : "NULL")
                  << "  ";
    }
    std::cout << "\n";
    return 0;
}

// ── Helper: run any SQL and print results ───────────────────────────────────
bool runQuery(sqlite3* db, const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), printRowCallback, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// ── Insert a course ─────────────────────────────────────────────────────────
bool insertCourse(sqlite3* db,
                  const std::string& courseId,
                  const std::string& courseName,
                  double priorityScore,
                  double difficultyScore,
                  bool juniorPlusStanding) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO courses (course_id, course, priority_score, "
        "difficulty_score, junior_plus_standing) VALUES (?,?,?,?,?);";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, courseId.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, courseName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, priorityScore);
    sqlite3_bind_double(stmt, 4, difficultyScore);
    sqlite3_bind_int(stmt,   5, juniorPlusStanding ? 1 : 0);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// ── Fetch courses by junior+ standing ───────────────────────────────────────
void getByStanding(sqlite3* db, bool juniorPlus) {
    std::string sql =
        "SELECT * FROM courses WHERE junior_plus_standing = " +
        std::to_string(juniorPlus ? 1 : 0) + ";";
    std::cout << "\n── Courses (junior+=" << juniorPlus << ") ──\n";
    runQuery(db, sql);
}

// ── Fetch courses sorted by priority ────────────────────────────────────────
void getByPriority(sqlite3* db) {
    std::cout << "\n── Courses by Priority (desc) ──\n";
    runQuery(db, "SELECT * FROM courses ORDER BY priority_score DESC;");
}

// ── Fetch courses under a difficulty threshold ───────────────────────────────
void getUnderDifficulty(sqlite3* db, double maxDifficulty) {
    std::string sql =
        "SELECT * FROM courses WHERE difficulty_score <= " +
        std::to_string(maxDifficulty) + " ORDER BY difficulty_score;";
    std::cout << "\n── Courses with difficulty <= " << maxDifficulty << " ──\n";
    runQuery(db, sql);
}

// ── main ────────────────────────────────────────────────────────────────────
int main() {
    sqlite3* db;

    // Open (or create) the database file
    if (sqlite3_open("courses.db", &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    // Create table
    const char* createSQL = R"(
        CREATE TABLE IF NOT EXISTS courses (
            course_id            TEXT PRIMARY KEY,
            course               TEXT NOT NULL,
            priority_score       REAL NOT NULL,
            difficulty_score     REAL NOT NULL,
            junior_plus_standing INTEGER NOT NULL DEFAULT 0
        );
    )";
    runQuery(db, createSQL);

    // Seed some data (only if table is empty)
    runQuery(db, "INSERT OR IGNORE INTO courses VALUES "
                 "('CS101','Intro to CS',3.5,2.0,0),"
                 "('CS301','Algorithms',4.8,4.5,1),"
                 "('CS401','Operating Systems',4.2,4.8,1),"
                 "('MATH201','Discrete Math',3.9,3.7,0);");

    // ── Example queries ──
    getByPriority(db);
    getByStanding(db, true);   // junior+ required
    getByStanding(db, false);  // open to all
    getUnderDifficulty(db, 4.0);

    // ── Insert a new course ──
    insertCourse(db, "CS450", "Compilers", 4.6, 4.9, true);
    std::cout << "\n── After inserting CS450 ──\n";
    runQuery(db, "SELECT * FROM courses;");

    sqlite3_close(db);
    return 0;
}