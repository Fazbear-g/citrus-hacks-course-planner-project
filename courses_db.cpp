// ============================================================
// UCR BCOE Course Planner
// Usage: ./courses_db <MAJOR> [taken_courses_comma_separated]
// Example: ./courses_db CS CS010A,MATH009A,MATH009B
//
// Valid majors: CS, CE, EE, ME, CHEME, BIOE, ENVE, MSE, DS, ROBO
//
// Compile:
//   g++ courses_db.cpp -o courses_db \
//     -I/opt/homebrew/opt/sqlite/include \
//     -L/opt/homebrew/opt/sqlite/lib \
//     -lsqlite3 -std=c++17 -g
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <functional>
#include <sqlite3.h>

// ── Constants ─────────────────────────────────────────────────────────────────

const int    MAX_COURSES_PER_QUARTER    = 5;
const int    MAX_UNITS_PER_QUARTER      = 18;
const int    JUNIOR_PLUS_QUARTER        = 7;   // 1-indexed
const int    MAX_BREADTH_PER_QUARTER    = 2;
const int    MAX_QUARTERS               = 20;  // hard cap
const double MAX_DIFFICULTY_PER_QUARTER = 30.0;

const std::vector<std::string> VALID_MAJORS = {
    "CS", "CE", "EE", "ME", "CHEME", "BIOE", "ENVE", "MSE", "DS", "ROBO"
};

const std::unordered_map<std::string, std::string> MAJOR_NAMES = {
    {"CS",    "Computer Science"},
    {"CE",    "Computer Engineering"},
    {"EE",    "Electrical Engineering"},
    {"ME",    "Mechanical Engineering"},
    {"CHEME", "Chemical Engineering"},
    {"BIOE",  "Bioengineering"},
    {"ENVE",  "Environmental Engineering"},
    {"MSE",   "Materials Science & Engineering"},
    {"DS",    "Data Science"},
    {"ROBO",  "Robotics Engineering"}
};

// ── Data Structures ───────────────────────────────────────────────────────────

struct Course {
    std::string course_id;
    std::string course_name;
    int         units;
    std::string prerequisite;
    std::string major;
    std::string division;
    double      priority_score;
    double      difficulty_score;
    bool        junior_plus_standing;
    bool        taken;
};

bool isBreadth(const Course& c) {
    return c.course_id.find("BREADTH") != std::string::npos;
}

bool isTechElec(const Course& c) {
    return c.course_id.find("TECHELEC") != std::string::npos;
}

bool isFiller(const Course& c) {
    return isBreadth(c) || isTechElec(c);
}

// ── Database Helpers ──────────────────────────────────────────────────────────

void ensureStudentCoursesTable(sqlite3* db) {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS student_courses (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            student_id  TEXT NOT NULL,
            course_id   TEXT NOT NULL,
            taken       INTEGER NOT NULL DEFAULT 1,
            FOREIGN KEY (course_id) REFERENCES courses(course_id),
            UNIQUE(student_id, course_id)
        );
    )";
    char* errMsg = nullptr;
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);
}

std::unordered_set<std::string> loadTakenCourses(sqlite3* db, const std::string& studentId) {
    std::unordered_set<std::string> taken;
    const char* sql = "SELECT course_id FROM student_courses WHERE student_id = ? AND taken = 1;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, studentId.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        taken.insert(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    sqlite3_finalize(stmt);
    return taken;
}

void markCourseTaken(sqlite3* db, const std::string& studentId, const std::string& courseId) {
    const char* sql = R"(
        INSERT INTO student_courses (student_id, course_id, taken)
        VALUES (?, ?, 1)
        ON CONFLICT(student_id, course_id) DO UPDATE SET taken = 1;
    )";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, studentId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, courseId.c_str(),  -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void clearTakenCourses(sqlite3* db, const std::string& studentId) {
    const char* sql = "DELETE FROM student_courses WHERE student_id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, studentId.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Course> loadCourses(sqlite3* db,
                                const std::string& major,
                                const std::unordered_set<std::string>& takenIds)
{
    std::vector<Course> courses;
    const char* sql = R"(
        SELECT course_id, course, units, COALESCE(prerequisite, ''),
               major, division, priority_score, difficulty_score,
               junior_plus_standing
        FROM courses
        WHERE major = 'ALL'
           OR major = ?
           OR major LIKE ? || ',%'
           OR major LIKE '%,' || ? || ',%'
           OR major LIKE '%,' || ?
        ORDER BY course_id;
    )";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, major.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, major.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, major.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, major.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Course c;
        c.course_id            = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        c.course_name          = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.units                = sqlite3_column_int(stmt, 2);
        c.prerequisite         = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        c.major                = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        c.division             = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        c.priority_score       = sqlite3_column_double(stmt, 6);
        c.difficulty_score     = sqlite3_column_double(stmt, 7);
        c.junior_plus_standing = sqlite3_column_int(stmt, 8) == 1;
        c.taken                = takenIds.count(c.course_id) > 0;
        courses.push_back(c);
    }
    sqlite3_finalize(stmt);
    return courses;
}

// ── Earliest Quarter Calculation ──────────────────────────────────────────────

std::unordered_map<std::string, int> computeEarliestQuarter(
    const std::vector<Course>& courses)
{
    std::unordered_map<std::string, Course> courseMap;
    for (const auto& c : courses) courseMap[c.course_id] = c;

    std::unordered_map<std::string, int> earliest;

    std::function<int(const std::string&)> dfs = [&](const std::string& id) -> int {
        if (earliest.count(id)) return earliest[id];
        if (!courseMap.count(id)) return 0;

        const Course& c = courseMap[id];
        if (c.taken) { earliest[id] = -1; return -1; }

        int prereqDepth = 0;
        if (!c.prerequisite.empty() && courseMap.count(c.prerequisite)) {
            int pd = dfs(c.prerequisite);
            prereqDepth = (pd < 0) ? 0 : pd + 1;
        }

        int minQ = prereqDepth;
        if (c.junior_plus_standing)
            minQ = std::max(minQ, JUNIOR_PLUS_QUARTER - 1);

        earliest[id] = minQ;
        return minQ;
    };

    for (const auto& c : courses)
        if (!c.taken) dfs(c.course_id);

    return earliest;
}

// ── Quarter Assignment ────────────────────────────────────────────────────────

std::vector<std::vector<Course>> assignToQuarters(const std::vector<Course>& courses) {

    std::vector<std::vector<Course>> plan(MAX_QUARTERS);
    std::vector<int>    qUnits(MAX_QUARTERS, 0);
    std::vector<double> qDiff(MAX_QUARTERS, 0.0);
    std::vector<int>    qCount(MAX_QUARTERS, 0);
    std::vector<int>    qBreadth(MAX_QUARTERS, 0);
    // Track how many non-breadth courses are in each quarter
    std::vector<int>    qCore(MAX_QUARTERS, 0);

    std::unordered_map<std::string, Course> courseMap;
    for (const auto& c : courses) courseMap[c.course_id] = c;

    std::unordered_set<std::string> scheduled;
    for (const auto& c : courses)
        if (c.taken) scheduled.insert(c.course_id);

    std::unordered_map<std::string, int> courseQuarter;
    auto earliest = computeEarliestQuarter(courses);

    std::vector<Course> coreCourses, breadthCourses;
    for (const auto& c : courses) {
        if (c.taken) continue;
        if (isBreadth(c)) breadthCourses.push_back(c);
        else              coreCourses.push_back(c);
    }

    std::sort(coreCourses.begin(), coreCourses.end(),
        [&](const Course& a, const Course& b) {
            int ea = earliest.count(a.course_id) ? earliest[a.course_id] : 0;
            int eb = earliest.count(b.course_id) ? earliest[b.course_id] : 0;
            if (ea != eb) return ea < eb;
            if (a.priority_score != b.priority_score)
                return a.priority_score > b.priority_score;
            return a.difficulty_score < b.difficulty_score;
        });

    // ── Phase 1: Place core courses ───────────────────────────────────────────
    bool progress = true;
    std::unordered_set<std::string> placedSet;

    while (progress) {
        progress = false;
        for (const auto& c : coreCourses) {
            if (placedSet.count(c.course_id)) continue;

            bool prereqMet = c.prerequisite.empty() || scheduled.count(c.prerequisite);
            if (!prereqMet) continue;

            int minQ = earliest.count(c.course_id) ? earliest[c.course_id] : 0;
            if (!c.prerequisite.empty() && courseQuarter.count(c.prerequisite))
                minQ = std::max(minQ, courseQuarter[c.prerequisite] + 1);

            for (int q = minQ; q < MAX_QUARTERS; q++) {
                if (qCount[q] >= MAX_COURSES_PER_QUARTER)        continue;
                if (qUnits[q] + c.units > MAX_UNITS_PER_QUARTER)  continue;
                if (qCount[q] > 0 && qDiff[q] + c.difficulty_score > MAX_DIFFICULTY_PER_QUARTER) continue;

                plan[q].push_back(c);
                qUnits[q]   += c.units;
                qDiff[q]    += c.difficulty_score;
                qCount[q]++;
                qCore[q]++;
                scheduled.insert(c.course_id);
                courseQuarter[c.course_id] = q;
                placedSet.insert(c.course_id);
                progress = true;
                break;
            }
        }
    }

    // ── Phase 2: Fill gaps with breadth courses ───────────────────────────────
    // First pass: only fill quarters that already have core courses
    // Second pass: fill any remaining quarter including breadth-only ones
    for (const auto& b : breadthCourses) {
        bool placed = false;

        // First try: quarters with core courses (preferred)
        for (int q = 0; q < MAX_QUARTERS && !placed; q++) {
            if (qCore[q] == 0)                               continue; // no core courses
            if (qCount[q] >= MAX_COURSES_PER_QUARTER)        continue;
            if (qUnits[q] + b.units > MAX_UNITS_PER_QUARTER)  continue;
            if (qBreadth[q] >= MAX_BREADTH_PER_QUARTER)       continue;

            plan[q].push_back(b);
            qUnits[q]   += b.units;
            qDiff[q]    += b.difficulty_score;
            qCount[q]++;
            qBreadth[q]++;
            placed = true;
        }

        // Second try: any non-empty quarter
        if (!placed) {
            for (int q = 0; q < MAX_QUARTERS && !placed; q++) {
                if (qCount[q] == 0)                              continue; // skip empty
                if (qCount[q] >= MAX_COURSES_PER_QUARTER)        continue;
                if (qUnits[q] + b.units > MAX_UNITS_PER_QUARTER)  continue;
                if (qBreadth[q] >= MAX_BREADTH_PER_QUARTER)       continue;

                plan[q].push_back(b);
                qUnits[q]   += b.units;
                qDiff[q]    += b.difficulty_score;
                qCount[q]++;
                qBreadth[q]++;
                placed = true;
            }
        }

        // Last resort: pair up with other breadth courses after last quarter
        if (!placed) {
            int lastQ = 0;
            for (int q = MAX_QUARTERS - 1; q >= 0; q--) {
                if (qCount[q] > 0) { lastQ = q + 1; break; }
            }
            // Try to pair with the last breadth-only quarter first
            for (int q = 0; q < lastQ && !placed; q++) {
                if (qCore[q] > 0)                                continue; // skip core quarters
                if (qCount[q] >= MAX_COURSES_PER_QUARTER)        continue;
                if (qUnits[q] + b.units > MAX_UNITS_PER_QUARTER)  continue;
                if (qBreadth[q] >= MAX_BREADTH_PER_QUARTER)       continue;

                plan[q].push_back(b);
                qUnits[q]   += b.units;
                qDiff[q]    += b.difficulty_score;
                qCount[q]++;
                qBreadth[q]++;
                placed = true;
            }
            // Truly last resort: new quarter
            if (!placed && lastQ < MAX_QUARTERS) {
                plan[lastQ].push_back(b);
                qUnits[lastQ]   += b.units;
                qDiff[lastQ]    += b.difficulty_score;
                qCount[lastQ]++;
                qBreadth[lastQ]++;
            }
        }
    }

    // ── Phase 3: Remove empty quarters ───────────────────────────────────────
    plan.erase(std::remove_if(plan.begin(), plan.end(),
        [](const std::vector<Course>& q) { return q.empty(); }), plan.end());

    return plan;
}

// ── Display ───────────────────────────────────────────────────────────────────

void printPlan(
    const std::vector<std::vector<Course>>& plan,
    const std::vector<Course>& takenCourses,
    const std::string& major)
{
    std::string majorName = MAJOR_NAMES.count(major) ? MAJOR_NAMES.at(major) : major;

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║          UCR COURSE PLAN — "
              << std::left << std::setw(24) << majorName << "║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n";

    if (!takenCourses.empty()) {
        int takenUnits = 0;
        for (const auto& c : takenCourses) takenUnits += c.units;
        std::cout << "\n── Already Completed [" << takenUnits << " units] ──\n";
        for (const auto& c : takenCourses) {
            std::cout << "  ✓ " << std::left << std::setw(12) << c.course_id
                      << " | " << c.course_name
                      << " (" << c.units << " units)\n";
        }
    }

    for (int q = 0; q < (int)plan.size(); q++) {
        double totalDiff = 0.0;
        int    totalUnits = 0;
        for (const auto& c : plan[q]) {
            totalDiff  += c.difficulty_score;
            totalUnits += c.units;
        }
        double avgDiff = plan[q].empty() ? 0.0 : totalDiff / plan[q].size();

        std::cout << "\n── Quarter " << (q + 1)
                  << "  [" << totalUnits << " units | avg difficulty: "
                  << std::fixed << std::setprecision(1) << avgDiff << "/10.0] ──\n";

        for (const auto& c : plan[q]) {
            std::cout << "  " << std::left << std::setw(12) << c.course_id
                      << " | " << std::setw(42) << c.course_name
                      << " | " << c.units << " units"
                      << " | diff: " << std::fixed << std::setprecision(1) << c.difficulty_score;
            if (c.junior_plus_standing) std::cout << "  [Junior+]";
            std::cout << "\n";
        }
    }

    int totalCourses = 0, totalUnits = 0;
    for (const auto& q : plan) {
        totalCourses += q.size();
        for (const auto& c : q) totalUnits += c.units;
    }
    int takenUnits = 0;
    for (const auto& c : takenCourses) takenUnits += c.units;

    std::cout << "\n── Summary ──────────────────────────────────────────\n";
    std::cout << "  Major          : " << majorName                  << "\n";
    std::cout << "  Units completed: " << takenUnits                 << "\n";
    std::cout << "  Units remaining: " << totalUnits                 << "\n";
    std::cout << "  Total units    : " << (takenUnits + totalUnits)  << "\n";
    std::cout << "  Quarters left  : " << plan.size()                << "\n";
    std::cout << "  Courses left   : " << totalCourses               << "\n\n";
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: ./courses_db <MAJOR> [taken_courses_comma_separated]\n";
        std::cerr << "Example: ./courses_db CS CS010A,MATH009A,MATH009B\n";
        std::cerr << "Valid majors: CS, CE, EE, ME, CHEME, BIOE, ENVE, MSE, DS, ROBO\n";
        return 1;
    }

    std::string major = argv[1];
    for (auto& ch : major) ch = toupper(ch);

    bool validMajor = false;
    for (const auto& m : VALID_MAJORS) if (m == major) { validMajor = true; break; }
    if (!validMajor) {
        std::cerr << "Invalid major: " << major << "\n";
        std::cerr << "Valid majors: CS, CE, EE, ME, CHEME, BIOE, ENVE, MSE, DS, ROBO\n";
        return 1;
    }

    sqlite3* db;
    if (sqlite3_open("courses.db", &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    ensureStudentCoursesTable(db);

    std::string studentId = "default";

    clearTakenCourses(db, studentId);
    if (argc >= 3) {
        std::stringstream ss(argv[2]);
        std::string courseId;
        while (std::getline(ss, courseId, ','))
            if (!courseId.empty()) markCourseTaken(db, studentId, courseId);
    }

    std::unordered_set<std::string> takenIds = loadTakenCourses(db, studentId);
    std::vector<Course> courses = loadCourses(db, major, takenIds);

    std::vector<Course> takenCourses;
    for (const auto& c : courses) if (c.taken) takenCourses.push_back(c);

    std::vector<std::vector<Course>> plan = assignToQuarters(courses);
    printPlan(plan, takenCourses, major);

    sqlite3_close(db);
    return 0;
}