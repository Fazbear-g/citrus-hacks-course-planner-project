#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "course_graph.h"    // your class
#include "directed_graph.h"  // your graph

struct IndexCoursePair {
  int index;
  float squared_diff;
  Course course;

  IndexCoursePair(int index_, float squared_diff_, Course course_)
      : index(index_), squared_diff(squared_diff_), course(course_) {}
};

const int MAX_COURSES_PER_QUARTER = 4;

void printCoursePlan(const std::vector<std::vector<Course>>& plan, int termsPerYear);

std::vector<std::vector<Course>> buildFullTerm(std::vector<std::vector<Course>> courses);
std::vector<int> build_indexes(int max);
void backtrack(int start,
               int k,
               const std::vector<int>& nums,
               std::vector<int>& current,
               std::vector<std::vector<int>>& result);
std::vector<std::vector<int>> combinations(const std::vector<int>& nums, int k);

int main() {
  // 📚 Create course database
  std::unordered_map<std::string, Course> courseMap = {
      {"CS010A", {"CS010A", "Intro to CS", 4, "", "CS", "lower", 1.0, 2.0, false, false}},
      {"CS010B", {"CS010B", "Data Structures", 4, "CS010A", "CS", "lower", 2.0, 3.0, false, false}},
      {"CS010C",
       {"CS010C", "Software Construction", 4, "CS010B", "CS", "lower", 3.0, 4.0, false, false}},
      {"CS011",
       {"CS011", "Discrete Structures", 4, "CS010B", "CS", "lower", 2.5, 3.5, false, false}},
      {"CS100", {"CS100", "Algorithms", 4, "CS010C", "CS", "upper", 5.0, 5.0, true, false}},
      {"CS120A",
       {"CS120A", "Operating Systems", 4, "CS100", "CS", "upper", 4.5, 5.0, true, false}}};

  // 🧠 Build graph starting from a course
  CourseGraph cg;
  cg.build(courseMap, "CS010B");

  // 🔍 Print graph structure
  std::cout << "===== GRAPH STRUCTURE =====\n";
  cg.printGraph();

  // 🔄 Topological sort
  std::cout << "\n===== COURSE ORDER =====\n";
  std::vector<Course> order = cg.topologicalSort();

  for (const Course& c : order) {
    std::cout << c.course_id << " - " << c.course_name << " | priority: " << c.priority_score
              << "\n";
  }

  return 0;

  std::vector<std::vector<Course>> full_list;
  full_list.push_back(order);

  std::vector<std::vector<Course>> full_term = buildFullTerm(full_list);

  printCoursePlan(full_term, 3);
}

void printCoursePlan(const std::vector<std::vector<Course>>& plan, int termsPerYear) {
  if (termsPerYear <= 0) {
    std::cout << "Invalid terms per year.\n";
    return;
  }

  const std::vector<std::string> termNames3 = {"Fall", "Winter", "Spring"};
  const std::vector<std::string> termNames4 = {"Fall", "Winter", "Spring", "Summer"};

  const std::vector<std::string>& termNames = (termsPerYear == 4) ? termNames4 : termNames3;

  std::cout << "==== COURSE PLAN ====\n\n";

  for (size_t i = 0; i < plan.size(); i++) {
    int year = i / termsPerYear + 1;
    int term = i % termsPerYear;

    std::cout << "Year " << year << " - " << termNames[term % termNames.size()]
              << "\n----------------------\n";

    if (plan[i].empty()) {
      std::cout << "  (No courses)\n";
    } else {
      for (const auto& course : plan[i]) {
        std::cout << "  " << course.course_id << " - " << course.course_name << " (" << course.units
                  << " units)\n";
      }
    }

    std::cout << "\n";
  }
}

std::vector<std::vector<Course>> buildFullTerm(std::vector<std::vector<Course>> courses) {
  std::vector<std::vector<Course>> full_term;
  // used for indicing through each list
  std::vector<size_t> indices(courses.size(), 0);

  std::unordered_set<std::string> takenCourses;

  size_t total_difficulty = 0;
  size_t count = 0;
  for (unsigned i = 0; i < courses.size(); i++) {
    for (unsigned j = 0; j < courses[i].size(); j++) {
      total_difficulty += courses[i][j].difficulty_score;
      count++;
    }
  }
  if (count == 0) {
    throw std::invalid_argument("Division by zero");
  }
  float average_difficulty = (float)total_difficulty / count;
  bool finished = false;
  // assembles X class size terms until there are no more classes left
  // TAKE INTO ACCOUNT BREADTH/TECH ELECTIVES
  // MINIMIZE (Average Difficulty - Term Difficulty)^2
  while (!finished) {
    std::vector<IndexCoursePair> current_classes;

    for (unsigned i = 0; i < courses.size(); i++) {
      if (indices[i] < courses[i].size()) {
        IndexCoursePair temp = IndexCoursePair(i, 0.0, courses[i][indices[i]]);
        // current_classes.push_back(courses[i][indices[i]]);
        current_classes.push_back(temp);
      }
    }
    if (current_classes.empty()) {  // no classes left
      finished = true;
      break;
    }

    std::pair<float, std::vector<int>> best_combination;

    size_t min_size = std::min((size_t)MAX_COURSES_PER_QUARTER, current_classes.size());
    if (current_classes.size() > MAX_COURSES_PER_QUARTER) {
      std::vector<int> indexes = build_indexes(current_classes.size());
      std::vector<std::vector<int>> all_combinations =
          combinations(indexes, MAX_COURSES_PER_QUARTER);

      float current_difficulty = 0.0;
      for (unsigned i = 0; i < MAX_COURSES_PER_QUARTER; i++) {
        current_difficulty += current_classes[all_combinations[0][i]].course.difficulty_score;
      }
      float squared_diff =
          (average_difficulty - current_difficulty) * (average_difficulty - current_difficulty);

      best_combination.first = squared_diff;
      best_combination.second = all_combinations[0];

      // finds best possible solution. NOTE THIS IS EXPENSIVE BUT WE HAVE SMALL N so its fine(O(N
      // * 2^N) I think)
      for (unsigned i = 0; i < all_combinations.size(); i++) {
        squared_diff = 0.0;
        current_difficulty = 0.0;
        for (unsigned j = 0; j < MAX_COURSES_PER_QUARTER; j++) {
          current_difficulty += current_classes[all_combinations[i][j]].course.difficulty_score;
        }
        squared_diff =
            (average_difficulty - current_difficulty) * (average_difficulty - current_difficulty);
        if (squared_diff < best_combination.first) {
          best_combination.first = squared_diff;
          best_combination.second = all_combinations[i];
        }
      }
      // TODO: DOES NOT ACCOUNT FOR POSSIBLE BREADTH COURSES OF TECH ELECTIVES
    } else {
      std::vector<int> indexes = build_indexes(current_classes.size());
      best_combination.second = indexes;
    }

    std::vector<int> class_indices = best_combination.second;

    std::vector<Course> best_classes;

    for (unsigned i = 0; i < class_indices.size(); i++) {
      int index = class_indices[i];
      IndexCoursePair index_pair = current_classes[index];

      best_classes.push_back(index_pair.course);

      indices[index_pair.index]++;
    }
    full_term.push_back(best_classes);
  }

  return full_term;
}

std::vector<int> build_indexes(int max) {
  std::vector<int> values(max - 1, 0);
  for (unsigned i = 0; i < max; i++) {
    values[i] = i;
  }
  return values;
}

void backtrack(int start,
               int k,
               const std::vector<int>& nums,
               std::vector<int>& current,
               std::vector<std::vector<int>>& result) {
  // if we built a full combination
  if (current.size() == k) {
    result.push_back(current);
    return;
  }

  // try each remaining element
  for (int i = start; i < nums.size(); i++) {
    current.push_back(nums[i]);
    backtrack(i + 1, k, nums, current, result);
    current.pop_back();
  }
}

std::vector<std::vector<int>> combinations(const std::vector<int>& nums, int k) {
  std::vector<std::vector<int>> result;
  std::vector<int> current;

  backtrack(0, k, nums, current, result);

  return result;
}