#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "course_graph.h"
#include "course_plan.h"
#include "directed_graph.h"

struct IndexCoursePair {
  unsigned index;
  unsigned offset;
  Course course;

  IndexCoursePair(unsigned index_, unsigned offset_, Course course_)
      : index(index_), offset(offset_), course(course_) {}
};

const int MAX_COURSES_PER_QUARTER = 4;

const int JUNIOR_PLUS_QUARTER = 7;

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
      {"CS010A", {"CS010A", "Intro to CS", 4, "", "CS", "lower", 1.0, 2.0, false}},
      {"CS010B", {"CS010B", "Data Structures", 4, "CS010A", "CS", "lower", 2.0, 3.0, false}},

      // Branching from CS010B
      {"CS010C", {"CS010C", "Software Construction", 4, "CS010B", "CS", "lower", 3.0, 4.0, false}},
      {"CS011", {"CS011", "Discrete Structures", 4, "CS010B", "CS", "lower", 2.5, 3.5, false}},
      {"CS012", {"CS012", "Computer Organization", 4, "CS010B", "CS", "lower", 2.8, 3.8, false}},

      // More branching later
      {"CS100", {"CS100", "Algorithms", 4, "CS010C", "CS", "upper", 5.0, 5.0, true}},
      {"CS111", {"CS111", "Theory of Computation", 4, "CS011", "CS", "upper", 4.5, 4.8, true}},
      {"CS120A", {"CS120A", "Operating Systems", 4, "CS100", "CS", "upper", 4.5, 5.0, true}},
      {"CS130", {"CS130", "Computer Graphics", 4, "CS012", "CS", "upper", 4.0, 4.5, true}},
  };

  std::unordered_map<std::string, Course> physicsMap = {
      {"PHYS040A", {"PHYS040A", "Mechanics", 4, "", "PHYS", "lower", 1.5, 3.5, false}},

      {"PHYS040B",
       {"PHYS040B", "Electricity and Magnetism", 4, "PHYS040A", "PHYS", "lower", 2.0, 4.0, false}},
      {"PHYS040C",
       {"PHYS040C", "Waves and Modern Physics", 4, "PHYS040B", "PHYS", "lower", 2.5, 4.2, false}},

      // Branching at upper level
      {"PHYS100A",
       {"PHYS100A", "Advanced Mechanics", 4, "PHYS040C", "PHYS", "upper", 4.0, 4.8, true}},
      {"PHYS100B", {"PHYS100B", "Electrodynamics", 4, "PHYS100A", "PHYS", "upper", 4.5, 5.0, true}},
      {"PHYS100C",
       {"PHYS100C", "Quantum Mechanics", 4, "PHYS100B", "PHYS", "upper", 5.0, 5.0, true}},

      {"PHYS105",
       {"PHYS105", "Computational Physics", 4, "PHYS040C", "PHYS", "upper", 4.2, 4.6, true}},
  };

  std::unordered_map<std::string, Course> mathMap = {
      {"MATH009A", {"MATH009A", "Calculus I", 4, "", "MATH", "lower", 1.0, 3.0, false}},
      {"MATH009B", {"MATH009B", "Calculus II", 4, "MATH009A", "MATH", "lower", 1.5, 3.5, false}},

      // 🔥 Branching here
      {"MATH009C", {"MATH009C", "Calculus III", 4, "MATH009B", "MATH", "lower", 2.0, 4.0, false}},
      {"MATH010A", {"MATH010A", "Linear Algebra", 4, "MATH009B", "MATH", "lower", 2.5, 3.8, false}},
      {"MATH011", {"MATH011", "Intro to Proofs", 4, "MATH009B", "MATH", "lower", 2.2, 3.6, false}},

      // Further progression
      {"MATH046",
       {"MATH046", "Differential Equations", 4, "MATH009C", "MATH", "lower", 3.0, 4.2, false}},
      {"MATH131", {"MATH131", "Real Analysis", 4, "MATH010A", "MATH", "upper", 4.5, 5.0, true}},
      {"MATH135", {"MATH135", "Abstract Algebra", 4, "MATH011", "MATH", "upper", 4.5, 4.9, true}},
  };

  std::unordered_map<std::string, Course> writMap = {
      {"WRIT001", {"WRIT001", "Entry-Level Writing", 4, "", "WRIT", "lower", 1.0, 2.0, false}},
      {"WRIT002",
       {"WRIT002", "Intermediate Composition", 4, "WRIT001", "WRIT", "lower", 1.5, 2.5, false}},

      // Branching
      {"WRIT007", {"WRIT007", "Writing for STEM", 4, "WRIT002", "WRIT", "lower", 2.0, 2.8, false}},
      {"WRIT008", {"WRIT008", "Creative Writing", 4, "WRIT002", "WRIT", "lower", 2.0, 2.7, false}},
      {"WRIT100",
       {"WRIT100", "Advanced Composition", 4, "WRIT002", "WRIT", "upper", 3.0, 3.5, true}},
  };
  // 🧠 Build graph starting from a course
  CourseGraph cg;
  CourseGraph phys;
  CourseGraph math;
  CourseGraph writ;
  cg.build(courseMap, "CS010B");
  phys.build(physicsMap, "PHYS040A");
  math.build(mathMap, "MATH009B");
  writ.build(writMap, "WRIT001");

  // 🔍 Print graph structure
  std::cout << "===== GRAPH STRUCTURE =====\n";
  cg.printGraph();

  std::cout << "===== GRAPH STRUCTURE =====\n";
  phys.printGraph();
  std::cout << "===== GRAPH STRUCTURE =====\n";
  math.printGraph();
  std::cout << "===== GRAPH STRUCTURE =====\n";
  writ.printGraph();

  // 🔄 Topological sort
  std::cout << "\n===== COURSE ORDER =====\n";
  std::vector<Course> order1 = cg.topologicalSort();
  std::vector<Course> order2 = phys.topologicalSort();
  std::vector<Course> order3 = math.topologicalSort();
  std::vector<Course> order4 = writ.topologicalSort();

  for (const Course& c : order1) {
    std::cout << c.course_id << " - " << c.course_name << " | priority: " << c.priority_score
              << "\n";
  }
  for (const Course& c : order2) {
    std::cout << c.course_id << " - " << c.course_name << " | priority: " << c.priority_score
              << "\n";
  }
  for (const Course& c : order3) {
    std::cout << c.course_id << " - " << c.course_name << " | priority: " << c.priority_score
              << "\n";
  }
  for (const Course& c : order4) {
    std::cout << c.course_id << " - " << c.course_name << " | priority: " << c.priority_score
              << "\n";
  }

  std::vector<std::vector<Course>> full_list;
  full_list.push_back(order1);
  full_list.push_back(order2);
  full_list.push_back(order3);
  full_list.push_back(order4);

  std::vector<std::vector<Course>> full_term = buildFullTerm(full_list);

  printCoursePlan(full_term, 3);

  return 0;
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

    if (plan.at(i).empty()) {
      std::cout << "  (No courses)\n";
    } else {
      double totalDifficulty = 0.0;
      for (const auto& course : plan.at(i)) {
        std::cout << "  " << course.course_id << " - " << course.course_name << " (" << course.units
                  << " units)\n";
        totalDifficulty += course.difficulty_score;
      }

      double avgDifficulty = totalDifficulty / plan.at(i).size();
      std::cout << "  Avg Difficulty: " << std::fixed << std::setprecision(2) << avgDifficulty
                << "\n";
    }

    std::cout << "\n";
  }
}

std::vector<std::vector<Course>> buildFullTerm(std::vector<std::vector<Course>> courses) {
  // TODO: Does not account for multiple prerequisites.
  // TODO: Adding a course to current_classes does not check if a course has already been taken so
  // it could potentially duplicate. Aslo assumes that the lists of courses does not contain
  // duplicates which it could

  // TODO: Better optimize all combinations. Could prob become 2^n/2 or something better.

  // TODO: If courses don't change we could use a unique hashing function for already pre computed
  // majors. As in don't recompute and entire major if we already have. MUST UPDATE TO FOLLOW COURSE
  // CHANGES HOWEVER

  std::vector<std::vector<Course>> full_term;

  std::unordered_set<std::string> taken_classes;  // hashed by id
  std::vector<Course> best_classes;

  // HARDCODED FOR NOW
  int max_breadth_count = 8;
  int max_tech_elec = 6;

  int breadth_count = 0;
  int tech_count = 0;

  float total_difficulty = 0.0;
  size_t count = 0;
  for (unsigned i = 0; i < courses.size(); i++) {
    for (unsigned j = 0; j < courses.at(i).size(); j++) {
      total_difficulty += courses.at(i).at(j).difficulty_score;
      count++;
    }
  }
  if (count == 0) {
    std::cout << "Courses.size(): " << courses.size() << std::endl;
    for (unsigned i = 0; i < courses.size(); i++) {
      std::cout << "Courses[" << i << "].size(): " << courses.at(i).size() << std::endl;
    }
    std::cout << "Total Difficulty: " << total_difficulty << std::endl;
    throw std::invalid_argument("Division by zero");
  }
  float average_difficulty = total_difficulty / count;
  bool finished = false;
  // assembles X class size terms until there are no more classes left
  // TAKE INTO ACCOUNT BREADTH/TECH ELECTIVES
  // MINIMIZE (Average Difficulty - Term Difficulty)^2
  while (!finished) {
    std::vector<IndexCoursePair> current_classes;

    float term_difficulty = 0.0;

    for (unsigned i = 0; i < courses.size(); i++) {
      if (courses.at(i).size() > 0) {
        unsigned offset = 0;
        IndexCoursePair temp = IndexCoursePair(i, 0, courses.at(i).at(offset));
        current_classes.push_back(temp);
        term_difficulty += temp.course.difficulty_score;
        bool is_valid = true;
        offset++;
        while (offset < courses.at(i).size() && is_valid) {
          Course curr = courses.at(i).at(offset);
          if (taken_classes.count(curr.prerequisite) == 1) {  // student has taken the valid prereq
            IndexCoursePair new_class = IndexCoursePair(i, offset, curr);
            current_classes.push_back(new_class);
            term_difficulty += new_class.course.difficulty_score;
            offset++;
          } else {  // can't take class
            is_valid = false;
          }
        }
      }
    }
    if (current_classes.empty()) {  // no classes left
      finished = true;
      break;
    }
    term_difficulty = term_difficulty / current_classes.size();

    // inject breadth and tech electives into the equation
    if (term_difficulty > average_difficulty && breadth_count < max_breadth_count) {
      Course ge = makeGE();
      IndexCoursePair temp = IndexCoursePair(0, 0, ge);
      current_classes.push_back(temp);
    }
    if (full_term.size() > 7 && tech_count < max_tech_elec) {
      Course te = makeTechElective();
      IndexCoursePair temp = IndexCoursePair(0, 0, te);
      current_classes.push_back(temp);
    }

    std::pair<float, std::vector<int>> best_combination;

    if (current_classes.size() > MAX_COURSES_PER_QUARTER) {
      std::vector<int> indexes = build_indexes(current_classes.size());
      std::vector<std::vector<int>> all_combinations =
          combinations(indexes, MAX_COURSES_PER_QUARTER);

      float current_difficulty = 0.0;
      for (unsigned i = 0; i < MAX_COURSES_PER_QUARTER; i++) {
        current_difficulty +=
            current_classes.at(all_combinations.at(0).at(i)).course.difficulty_score;
      }
      current_difficulty = current_difficulty / MAX_COURSES_PER_QUARTER;

      float squared_diff =
          (average_difficulty - current_difficulty) * (average_difficulty - current_difficulty);

      best_combination.first = squared_diff;
      best_combination.second = all_combinations.at(0);

      // finds best possible solution. NOTE THIS IS EXPENSIVE BUT WE HAVE SMALL N so its fine(O(N
      // * 2^N) I think)
      for (unsigned i = 0; i < all_combinations.size(); i++) {
        squared_diff = 0.0;
        current_difficulty = 0.0;
        for (unsigned j = 0; j < MAX_COURSES_PER_QUARTER; j++) {
          current_difficulty +=
              current_classes.at(all_combinations.at(i).at(j)).course.difficulty_score;
        }
        current_difficulty = current_difficulty / MAX_COURSES_PER_QUARTER;

        squared_diff =
            (average_difficulty - current_difficulty) * (average_difficulty - current_difficulty);
        if (squared_diff < best_combination.first) {
          best_combination.first = squared_diff;
          best_combination.second = all_combinations.at(i);
        }
      }
    } else {
      std::vector<int> indexes = build_indexes(current_classes.size());
      best_combination.first = 0.0;
      best_combination.second = indexes;
      if (current_classes.size() < MAX_COURSES_PER_QUARTER) {
        float curr_diff = 0.0;
        count = 0;
        std::vector<int> class_i = best_combination.second;
        for (unsigned i = 0; i < class_i.size(); i++) {
          int index = class_i.at(i);
          IndexCoursePair index_pair = current_classes.at(index);

          curr_diff += index_pair.course.difficulty_score;
          count++;
        }
        if (count == 0) {
          std::cout << "Courses.size(): " << courses.size() << std::endl;
          for (unsigned i = 0; i < courses.size(); i++) {
            std::cout << "Courses[" << i << "].size(): " << courses.at(i).size() << std::endl;
          }
          std::cout << "Total Difficulty: " << total_difficulty << std::endl;
          throw std::invalid_argument("Division by zero");
        }
        float avg_term_diff = curr_diff / count;
        if (avg_term_diff > average_difficulty) {
          if (breadth_count < max_breadth_count) {
            Course ge = makeGE();
            best_classes.push_back(ge);
            breadth_count++;
          }
        } else if (avg_term_diff < average_difficulty && full_term.size() >= JUNIOR_PLUS_QUARTER) {
          if (tech_count < max_tech_elec) {
            Course te = makeTechElective();
            best_classes.push_back(te);
            tech_count++;
          }
        }
      }
    }

    std::vector<int> class_indices = best_combination.second;

    std::sort(class_indices.begin(), class_indices.end(), [&](int a, int b) {
      return current_classes.at(a).offset > current_classes.at(b).offset;
    });

    for (unsigned i = 0; i < class_indices.size(); i++) {
      int index = class_indices.at(i);
      IndexCoursePair index_pair = current_classes.at(index);

      best_classes.push_back(index_pair.course);

      // REMOVE BY GREATEST OFFSET FIRST OR THE LIST WOULD MOVE WHILE DELETEING = SEGFAULT

      if (index_pair.course.course_name != "Technical Elective" &&
          index_pair.course.course_name != "General Education") {
        auto& vec = courses.at(index_pair.index);

        if (index_pair.offset < vec.size()) {
          vec.erase(vec.begin() + index_pair.offset);
        }
        taken_classes.insert(index_pair.course.course_id);
      } else {
        if (index_pair.course.course_name == "Technical Elective") {
          tech_count++;
        } else {
          breadth_count++;
        }
      }
    }
    full_term.push_back(best_classes);
    best_classes.clear();
  }

  return full_term;
}

std::vector<int> build_indexes(int max) {
  std::vector<int> values(max);
  for (int i = 0; i < max; i++) {
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
    current.push_back(nums.at(i));
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