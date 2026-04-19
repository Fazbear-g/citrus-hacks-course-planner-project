#include <iostream>
#include <string>
#include <vector>

struct Course {
  std::string course_id;
  std::string course_name;
  int units;
  std::string prerequisite;
  std::string major;
  std::string division;  // upper/lower division
  double priority_score;
  double difficulty_score;
  bool junior_plus_standing;
  bool taken;
};
