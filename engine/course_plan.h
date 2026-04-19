#ifndef COURSE_PLAN_H
#define COURSE_PLAN_H

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
};

Course makeTechElective(const std::string& id = "TECH_ELEC") {
  return Course{
      id,
      "Technical Elective",
      4,
      "",       // no prerequisite
      "CS",     // or "TECH"
      "upper",  // usually upper division
      0.5,      // higher than GE, lower than core classes
      3.5,      // higher difficulty
      true,     // usually requires junior standing
  };
}

Course makeGE(const std::string& id = "GE_ANY") {
  return Course{
      id,
      "General Education",
      4,
      "",
      "GE",
      "any",
      0.0,
      1.0,
      false,
  };
}

#endif
