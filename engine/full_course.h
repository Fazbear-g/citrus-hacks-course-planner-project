// // ============================================================
// // UCR BCOE Course Planner
// // Usage: ./courses_db <MAJOR>
// // Example: ./courses_db CS
// //
// // Valid majors: CS, CE, EE, ME, CHEME, BIOE, ENVE, MSE, DS, ROBO
// //
// // Compile:
// //   g++ courses_db.cpp -o courses_db \
// //     -I/opt/homebrew/opt/sqlite/include \
// //     -L/opt/homebrew/opt/sqlite/lib \
// //     -lsqlite3 -std=c++17 -g
// // ============================================================

// #include <sqlite3.h>

// #include <algorithm>
// #include <cmath>
// #include <iomanip>
// #include <iostream>
// #include <numeric>
// #include <queue>
// #include <string>
// #include <unordered_map>
// #include <unordered_set>
// #include <utility>
// #include <vector>

// #include "course_graph.h"
// #include "course_plan.h"
// #include "directed_graph.h"

// // ── Constants ─────────────────────────────────────────────────────────────────

// const int MAX_COURSES_PER_QUARTER = 4;
// const int MAX_UNITS_PER_QUARTER = 20;
// const int JUNIOR_PLUS_QUARTER = 7;        // quarters 7+ allow junior+ courses
// const int MAX_TECH_ELEC_PER_QUARTER = 2;  // max 2 tech elective per quarter
// const int MAX_BREADTH_PER_QUARTER = 2;    // max 2 breadth per quarter

// struct IndexCoursePair {
//   int index;
//   float squared_diff;
//   Course course;

//   IndexCoursePair(int index_, float squared_diff_, Course course_)
//       : index(index_), squared_diff(squared_diff_), course(course_) {}
// }

// const std::vector<std::string>
//     VALID_MAJORS = {"CS", "CE", "EE", "ME", "CHEME", "BIOE", "ENVE", "MSE", "DS", "ROBO"};

// const std::unordered_map<std::string, std::string> MAJOR_NAMES = {
//     {"CS", "Computer Science"},
//     {"CE", "Computer Engineering"},
//     {"EE", "Electrical Engineering"},
//     {"ME", "Mechanical Engineering"},
//     {"CHEME", "Chemical Engineering"},
//     {"BIOE", "Bioengineering"},
//     {"ENVE", "Environmental Engineering"},
//     {"MSE", "Materials Science & Engineering"},
//     {"DS", "Data Science"},
//     {"ROBO", "Robotics Engineering"}};

// // ── Data Structures ───────────────────────────────────────────────────────────

// bool isBreadth(const Course& c) { return c.course_id.find("BREADTH") != std::string::npos; }

// bool isTechElec(const Course& c) { return c.course_id.find("TECHELEC") != std::string::npos; }

// bool isFiller(const Course& c) { return isBreadth(c) || isTechElec(c); }

// // ── Database Helpers ──────────────────────────────────────────────────────────

// std::unordered_map<std::string, Course> loadCourseMap(sqlite3* db, const std::string& major) {
//   std::unordered_map<std::string, Course> courseMap;

//   std::string query =
//       "SELECT course_id, course_name, units, prerequisite, major, division, "
//       "priority_score, difficulty_score, junior_plus_standing, taken "
//       "FROM courses WHERE major = ?;";

//   sqlite3_stmt* stmt;

//   if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
//     std::cerr << "Failed to prepare statement\n";
//     return courseMap;
//   }

//   // Bind the major parameter
//   sqlite3_bind_text(stmt, 1, major.c_str(), -1, SQLITE_STATIC);

//   while (sqlite3_step(stmt) == SQLITE_ROW) {
//     Course c;

//     // Required text fields
//     c.course_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
//     c.course_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

//     // Integers
//     c.units = sqlite3_column_int(stmt, 2);

//     // Handle NULL prerequisite safely
//     const unsigned char* prereqText = sqlite3_column_text(stmt, 3);
//     c.prerequisite = prereqText ? reinterpret_cast<const char*>(prereqText) : "";

//     // More text fields
//     c.major = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
//     c.division = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

//     // Doubles
//     c.priority_score = sqlite3_column_double(stmt, 6);
//     c.difficulty_score = sqlite3_column_double(stmt, 7);

//     // Booleans
//     c.junior_plus_standing = sqlite3_column_int(stmt, 8) != 0;
//     // TODO: DONT USE TAKEN INSTEAD HAVE IT ONLY ON ALGORITHM SIDE
//     c.taken = sqlite3_column_int(stmt, 9) != 0;

//     // Insert into map
//     courseMap[c.course_id] = c;
//   }

//   sqlite3_finalize(stmt);
//   return courseMap;
// }

// // each row is the quarter/semester and each list is the classes taken that quarter/semester
// // takes a list of topologically sorted course lists(needs to be a list to fill all courses that
// // might not connect i.e physics and cs)
// std::vector<std::vector<Course>> buildFullTerm(std::vector<std::vector<Course>> courses) {
//   std::vector<std::vector<Course>> full_term;
//   // used for indicing through each list
//   std::vector<size_t> indices(courses.size(), 0);

//   std::unordered_set<std::string> takenCourses;

//   size_t total_difficulty = 0;
//   size_t count = 0;
//   for (unsigned i = 0; i < courses.size(); i++) {
//     for (unsigned j = 0; j < courses[i].size(); j++) {
//       total_difficulty += courses[i][j].difficulty_score;
//       count++;
//     }
//   }
//   if (count == 0) {
//     throw std::invalid_argument("Division by zero");
//   }
//   float average_difficulty = (float)total_difficulty / count;
//   bool finished = false;
//   // assembles X class size terms until there are no more classes left
//   // TAKE INTO ACCOUNT BREADTH/TECH ELECTIVES
//   // MINIMIZE (Average Difficulty - Term Difficulty)^2
//   while (!finished) {
//     std::vector<IndexCoursePair> current_classes;

//     for (unsigned i = 0; i < courses.size(); i++) {
//       if (indices[i] < courses[i].size()) {
//         IndexCoursePair temp = IndexCoursePair(i, 0.0, courses[i][indices[i]]);
//         // current_classes.push_back(courses[i][indices[i]]);
//         current_classes.push_back(temp);
//       }
//     }
//     if (current_classes.empty()) {  // no classes left
//       finished = true;
//       break;
//     }

//     std::pair<float, std::vector<int>> best_combination;

//     size_t min_size = std::min(MAX_COURSES_PER_QUARTER, current_classes.size());
//     if (current_classes.size() > MAX_COURSES_PER_QUARTER) {
//       std::vector<int> indexes = build_indexes(current_classes.size());
//       std::vector<std::vector<int>> all_combinations =
//           combinations(indexes, MAX_COURSES_PER_QUARTER);

//       float current_difficulty = 0.0;
//       for (unsigned i = 0; i < MAX_COURSES_PER_QUARTER; i++) {
//         current_difficulty += current_classes[all_combinations[0][i]].course.difficulty_score;
//       }
//       float squared_diff =
//           (average_difficulty - current_difficulty) * (average_difficulty - current_difficulty);

//       best_combination.first = squared_diff;
//       best_combination.second = all_combinations[0];

//       // finds best possible solution. NOTE THIS IS EXPENSIVE BUT WE HAVE SMALL N so its fine(O(N
//       // * 2^N) I think)
//       for (unsigned i = 0; i < all_combinations.size(); i++) {
//         squared_diff = 0.0;
//         current_difficulty = 0.0;
//         for (unsigned j = 0; j < MAX_COURSES_PER_QUARTER; j++) {
//           current_difficulty += current_classes[all_combinations[i][j]].course.difficulty_score;
//         }
//         squared_diff =
//             (average_difficulty - current_difficulty) * (average_difficulty -
//             current_difficulty);
//         if (squared_diff < best_combination.first) {
//           best_combination.first = squared_diff;
//           best_combination.second = all_combinations[i];
//         }
//       }
//       // TODO: DOES NOT ACCOUNT FOR POSSIBLE BREADTH COURSES OF TECH ELECTIVES
//     } else {
//       std::vector<int> indexes = build_max(current_classes.size());
//       best_combination.second = indexes;
//     }

//     std::vector<int> class_indices = best_combination.second;

//     std::vector<Course> best_classes;

//     for (unsigned i = 0; i < class_indices.size(); i++) {
//       int index = class_indices[i];
//       IndexCoursePair index_pair = current_classes[index];

//       best_classes.push_back(index_pair.course);

//       indices[index_pair.index]++;
//     }
//     full_term.push_back(best_classes);
//   }

//   return full_term;
// }

// std::vector<int> build_indexes(int max) {
//   vector<int> values(max - 1, 0);
//   for (unsigned i = 0; i < max; i++) {
//     values[i] = i;
//   }
// }

// void backtrack(int start,
//                int k,
//                const std::vector<int>& nums,
//                std::vector<int>& current,
//                std::vector<std::vector<int>>& result) {
//   // if we built a full combination
//   if (current.size() == k) {
//     result.push_back(current);
//     return;
//   }

//   // try each remaining element
//   for (int i = start; i < nums.size(); i++) {
//     current.push_back(nums[i]);
//     backtrack(i + 1, k, nums, current, result);
//     current.pop_back();
//   }
// }

// std::vector<std::vector<int>> combinations(const std::vector<int>& nums, int k) {
//   std::vector<std::vector<int>> result;
//   std::vector<int> current;

//   backtrack(0, k, nums, current, result);

//   return result;
// }

// std::vector<Course> loadCourses(sqlite3* db, const std::string& major) {
//   std::vector<Course> courses;
//   const char* sql = R"(
//         SELECT course_id, course, units, COALESCE(prerequisite, ''),
//                major, division, priority_score, difficulty_score,
//                junior_plus_standing, taken
//         FROM courses
//         WHERE major = 'ALL'
//            OR major = ?
//            OR major LIKE ? || ',%'
//            OR major LIKE '%,' || ? || ',%'
//            OR major LIKE '%,' || ?
//         ORDER BY course_id;
//     )";
//   sqlite3_stmt* stmt;
//   sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
//   sqlite3_bind_text(stmt, 1, major.c_str(), -1, SQLITE_STATIC);
//   sqlite3_bind_text(stmt, 2, major.c_str(), -1, SQLITE_STATIC);
//   sqlite3_bind_text(stmt, 3, major.c_str(), -1, SQLITE_STATIC);
//   sqlite3_bind_text(stmt, 4, major.c_str(), -1, SQLITE_STATIC);
//   while (sqlite3_step(stmt) == SQLITE_ROW) {
//     Course c;
//     c.course_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
//     c.course_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
//     c.units = sqlite3_column_int(stmt, 2);
//     c.prerequisite = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
//     c.major = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
//     c.division = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
//     c.priority_score = sqlite3_column_double(stmt, 6);
//     c.difficulty_score = sqlite3_column_double(stmt, 7);
//     c.junior_plus_standing = sqlite3_column_int(stmt, 8) == 1;
//     c.taken = sqlite3_column_int(stmt, 9) == 1;
//     courses.push_back(c);
//   }
//   sqlite3_finalize(stmt);
//   return courses;
// }

// bool markCourseTaken(sqlite3* db, const std::string& courseId, bool taken) {
//   sqlite3_stmt* stmt;
//   const char* sql = "UPDATE courses SET taken = ? WHERE course_id = ?;";
//   sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
//   sqlite3_bind_int(stmt, 1, taken ? 1 : 0);
//   sqlite3_bind_text(stmt, 2, courseId.c_str(), -1, SQLITE_STATIC);
//   int rc = sqlite3_step(stmt);
//   sqlite3_finalize(stmt);
//   return rc == SQLITE_DONE;
// }

// void resetAllTaken(sqlite3* db) {
//   char* errMsg = nullptr;
//   sqlite3_exec(db, "UPDATE courses SET taken = 0;", nullptr, nullptr, &errMsg);
//   if (errMsg) sqlite3_free(errMsg);
// }

// // ── Step 1: Compute earliest possible quarter for each course ─────────────────
// // This gives every course a "depth" in the prerequisite chain.
// // A course can only be taken after all its prerequisites are done.

// std::unordered_map<std::string, int> computeEarliestQuarter(const std::vector<Course>& courses) {
//   std::unordered_map<std::string, Course> courseMap;
//   for (const auto& c : courses) courseMap[c.course_id] = c;

//   std::unordered_map<std::string, int> earliest;

//   // Recursive DFS with memoization
//   std::function<int(const std::string&)> dfs = [&](const std::string& id) -> int {
//     if (earliest.count(id)) return earliest[id];
//     if (!courseMap.count(id)) return 0;

//     const Course& c = courseMap[id];

//     // Already taken courses contribute 0 depth
//     if (c.taken) {
//       earliest[id] = 0;
//       return 0;
//     }

//     int prereqDepth = 0;
//     if (!c.prerequisite.empty() && courseMap.count(c.prerequisite)) {
//       if (courseMap[c.prerequisite].taken) {
//         prereqDepth = 0;  // prereq done, this course is unlocked at depth 1
//       } else {
//         prereqDepth = dfs(c.prerequisite) + 1;
//       }
//     }

//     // Junior+ courses can't start before quarter 6 (0-indexed)
//     int minQuarter = prereqDepth;
//     if (c.junior_plus_standing) minQuarter = std::max(minQuarter, JUNIOR_PLUS_QUARTER - 1);

//     // Filler courses: breadth can go anywhere, tech electives only junior+
//     // (already handled by junior_plus_standing flag)

//     earliest[id] = minQuarter;
//     return minQuarter;
//   };

//   for (const auto& c : courses) {
//     if (!c.taken) dfs(c.course_id);
//   }

//   return earliest;
// }

// // ── Step 2: Assign courses to quarters ───────────────────────────────────────
// // Groups courses by their earliest quarter, then balances difficulty
// // by redistributing if a quarter is overloaded.

// std::vector<std::vector<Course>> assignToQuarters(const std::vector<Course>& courses) {
//   // Build map for lookup
//   std::unordered_map<std::string, Course> courseMap;
//   for (const auto& c : courses)
//     if (!c.taken) courseMap[c.course_id] = c;

//   // Get earliest quarter for each course
//   auto earliest = computeEarliestQuarter(courses);

//   // Collect non-taken courses
//   std::vector<Course> remaining;
//   for (const auto& c : courses) {
//     if (!c.taken) remaining.push_back(c);
//   }

//   // Separate fillers from core courses
//   std::vector<Course> coreCourses, breadthCourses, techElecCourses;
//   for (const auto& c : remaining) {
//     if (isTechElec(c))
//       techElecCourses.push_back(c);
//     else if (isBreadth(c))
//       breadthCourses.push_back(c);
//     else
//       coreCourses.push_back(c);
//   }

//   // ── Phase 1: Schedule core courses first ──
//   // Sort core courses by (earliest quarter ASC, priority DESC, difficulty ASC)
//   std::sort(coreCourses.begin(), coreCourses.end(), [&](const Course& a, const Course& b) {
//     int ea = earliest.count(a.course_id) ? earliest[a.course_id] : 0;
//     int eb = earliest.count(b.course_id) ? earliest[b.course_id] : 0;
//     if (ea != eb) return ea < eb;
//     if (a.priority_score != b.priority_score) return a.priority_score > b.priority_score;
//     return a.difficulty_score < b.difficulty_score;
//   });

//   // Build plan quarter by quarter
//   // Each entry: list of courses in that quarter
//   std::vector<std::vector<Course>> plan;
//   std::unordered_set<std::string> scheduled;

//   // Helper: find which quarter a course is currently in
//   auto getQuarterOf = [&](const std::string& id) -> int {
//     for (int q = 0; q < (int)plan.size(); q++)
//       for (const auto& c : plan[q])
//         if (c.course_id == id) return q;
//     return -1;
//   };

//   // Schedule core courses
//   std::vector<Course> unscheduled = coreCourses;
//   int maxPasses = 30;

//   while (!unscheduled.empty() && maxPasses-- > 0) {
//     bool anyScheduled = false;

//     for (auto it = unscheduled.begin(); it != unscheduled.end();) {
//       const Course& c = *it;
//       int minQ = earliest.count(c.course_id) ? earliest[c.course_id] : 0;

//       // Check prereq is scheduled
//       bool prereqMet = c.prerequisite.empty() || scheduled.count(c.prerequisite);
//       if (!prereqMet) {
//         ++it;
//         continue;
//       }

//       // If prereq is scheduled, must come after it
//       if (!c.prerequisite.empty() && scheduled.count(c.prerequisite)) {
//         int prereqQ = getQuarterOf(c.prerequisite);
//         if (prereqQ >= 0) minQ = std::max(minQ, prereqQ + 1);
//       }

//       // Find the first quarter >= minQ that has room
//       bool placed = false;
//       for (int q = minQ; q < (int)plan.size() + 1; q++) {
//         // Expand plan if needed
//         while ((int)plan.size() <= q) plan.push_back({});

//         int courseCount = (int)plan[q].size();
//         int unitCount = 0;
//         for (const auto& x : plan[q]) unitCount += x.units;

//         if (courseCount < MAX_COURSES_PER_QUARTER && unitCount + c.units <=
//         MAX_UNITS_PER_QUARTER) {
//           plan[q].push_back(c);
//           scheduled.insert(c.course_id);
//           it = unscheduled.erase(it);
//           anyScheduled = true;
//           placed = true;
//           break;
//         }
//       }
//       if (!placed) ++it;
//     }

//     if (!anyScheduled) break;  // safety
//   }

//   // ── Phase 2: Spread breadth courses evenly ──
//   // Place 1 breadth per quarter starting from quarter 0
//   {
//     int qi = 0;
//     for (const auto& b : breadthCourses) {
//       // Find a quarter with room, starting from qi
//       bool placed = false;
//       for (int q = qi; q < (int)plan.size() + 8; q++) {
//         while ((int)plan.size() <= q) plan.push_back({});

//         int courseCount = (int)plan[q].size();
//         int unitCount = 0;
//         int breadthCount = 0;
//         for (const auto& x : plan[q]) {
//           unitCount += x.units;
//           if (isBreadth(x)) breadthCount++;
//         }

//         if (courseCount < MAX_COURSES_PER_QUARTER && unitCount + b.units <= MAX_UNITS_PER_QUARTER
//         &&
//             breadthCount < MAX_BREADTH_PER_QUARTER) {
//           plan[q].push_back(b);
//           scheduled.insert(b.course_id);
//           qi = q + 1;  // next breadth goes in next quarter or later
//           placed = true;
//           break;
//         }
//       }
//       if (!placed) {
//         // Force into a new quarter at the end
//         plan.push_back({b});
//         scheduled.insert(b.course_id);
//       }
//     }
//   }

//   // ── Phase 3: Spread tech electives evenly across junior+ quarters ──
//   {
//     int qi = JUNIOR_PLUS_QUARTER - 1;  // start at quarter 7 (0-indexed: 6)
//     for (const auto& te : techElecCourses) {
//       bool placed = false;
//       for (int q = qi; q < (int)plan.size() + 8; q++) {
//         while ((int)plan.size() <= q) plan.push_back({});

//         int courseCount = (int)plan[q].size();
//         int unitCount = 0;
//         int techCount = 0;
//         for (const auto& x : plan[q]) {
//           unitCount += x.units;
//           if (isTechElec(x)) techCount++;
//         }

//         if (courseCount < MAX_COURSES_PER_QUARTER &&
//             unitCount + te.units <= MAX_UNITS_PER_QUARTER &&
//             techCount < MAX_TECH_ELEC_PER_QUARTER) {
//           plan[q].push_back(te);
//           scheduled.insert(te.course_id);
//           qi = q + 1;
//           placed = true;
//           break;
//         }
//       }
//       if (!placed) {
//         plan.push_back({te});
//         scheduled.insert(te.course_id);
//       }
//     }
//   }

//   // ── Phase 4: Remove any empty quarters ──
//   plan.erase(std::remove_if(plan.begin(),
//                             plan.end(),
//                             [](const std::vector<Course>& q) { return q.empty(); }),
//              plan.end());

//   return plan;
// }

// // ── Display ───────────────────────────────────────────────────────────────────

// void printPlan(const std::vector<std::vector<Course>>& plan,
//                const std::vector<Course>& takenCourses,
//                const std::string& major) {
//   std::string majorName = MAJOR_NAMES.count(major) ? MAJOR_NAMES.at(major) : major;

//   std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
//   std::cout << "║          UCR COURSE PLAN — " << std::left << std::setw(24) << majorName <<
//   "║\n"; std::cout << "╚══════════════════════════════════════════════════════╝\n";

//   // Show completed courses
//   if (!takenCourses.empty()) {
//     int takenUnits = 0;
//     for (const auto& c : takenCourses) takenUnits += c.units;
//     std::cout << "\n── Already Completed [" << takenUnits << " units] ──\n";
//     for (const auto& c : takenCourses) {
//       std::cout << "  ✓ " << std::left << std::setw(12) << c.course_id << " | " << c.course_name
//                 << " (" << c.units << " units)\n";
//     }
//   }

//   // Show planned quarters
//   for (int q = 0; q < (int)plan.size(); q++) {
//     double totalDiff = 0.0;
//     int totalUnits = 0;
//     for (const auto& c : plan[q]) {
//       totalDiff += c.difficulty_score;
//       totalUnits += c.units;
//     }
//     double avgDiff = plan[q].empty() ? 0.0 : totalDiff / plan[q].size();

//     std::cout << "\n── Quarter " << (q + 1) << "  [" << totalUnits
//               << " units | avg difficulty: " << std::fixed << std::setprecision(1) << avgDiff
//               << "/10.0] ──\n";

//     for (const auto& c : plan[q]) {
//       std::cout << "  " << std::left << std::setw(12) << c.course_id << " | " << std::setw(42)
//                 << c.course_name << " | " << c.units << " units"
//                 << " | diff: " << std::fixed << std::setprecision(1) << c.difficulty_score;
//       if (c.junior_plus_standing) std::cout << "  [Junior+]";
//       std::cout << "\n";
//     }
//   }

//   // Summary
//   int totalCourses = 0, totalUnits = 0;
//   for (const auto& q : plan) {
//     totalCourses += q.size();
//     for (const auto& c : q) totalUnits += c.units;
//   }
//   int takenUnits = 0;
//   for (const auto& c : takenCourses) takenUnits += c.units;

//   std::cout << "\n── Summary ──────────────────────────────────────────\n";
//   std::cout << "  Major          : " << majorName << "\n";
//   std::cout << "  Units completed: " << takenUnits << "\n";
//   std::cout << "  Units remaining: " << totalUnits << "\n";
//   std::cout << "  Total units    : " << (takenUnits + totalUnits) << "\n";
//   std::cout << "  Quarters left  : " << plan.size() << "\n";
//   std::cout << "  Courses left   : " << totalCourses << "\n\n";
// }

// // ── Main ──────────────────────────────────────────────────────────────────────

// int main(int argc, char* argv[]) {
//   if (argc < 2) {
//     std::cerr << "Usage: ./courses_db <MAJOR>\n";
//     std::cerr << "Valid majors: CS, CE, EE, ME, CHEME, BIOE, ENVE, MSE, DS, ROBO\n";
//     return 1;
//   }

//   std::string major = argv[1];
//   for (auto& ch : major) ch = toupper(ch);

//   bool validMajor = false;
//   for (const auto& m : VALID_MAJORS)
//     if (m == major) {
//       validMajor = true;
//       break;
//     }
//   if (!validMajor) {
//     std::cerr << "Invalid major: " << major << "\n";
//     std::cerr << "Valid majors: CS, CE, EE, ME, CHEME, BIOE, ENVE, MSE, DS, ROBO\n";
//     return 1;
//   }

//   sqlite3* db;
//   if (sqlite3_open("courses.db", &db) != SQLITE_OK) {
//     std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
//     return 1;
//   }

//   // Reset and mark taken courses (will come from frontend later)
//   resetAllTaken(db);
//   markCourseTaken(db, "MATH009A", true);
//   markCourseTaken(db, "MATH009B", true);
//   markCourseTaken(db, "CS010A", true);
//   markCourseTaken(db, "ENGR001I", true);

//   // Load courses
//   std::vector<Course> courses = loadCourses(db, major);

//   std::vector<Course> takenCourses;
//   for (const auto& c : courses)
//     if (c.taken) takenCourses.push_back(c);

//   // Build plan
//   std::vector<std::vector<Course>> plan = assignToQuarters(courses);

//   // Print
//   printPlan(plan, takenCourses, major);

//   sqlite3_close(db);
//   return 0;
// }
