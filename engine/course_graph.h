#pragma once
// ============================================================
// course_graph.h
// Builds a directed prerequisite graph from a starting course.
// Depends on directed_graph.h which provides:
//   addNode(node)
//   addEdge(from, to)
//   getNeighbors(node)  -> vector of nodes reachable FROM node
//   getInDegree(node)   -> number of incoming edges
//   hasNode(node)       -> bool, O(1)
// ============================================================

#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "course_plan.h"
#include "directed_graph.h"

// ── Course struct ─────────────────────────────────────────────────────────────

// ── CourseGraph ───────────────────────────────────────────────────────────────
// Wraps directed_graph and adds course-specific logic.
// Edge direction: prerequisite → course
//   (meaning: take the prereq before you can take the course)

class CourseGraph {
 private:
  DirectedGraph<std::string> graph_;
  std::unordered_set<std::string> nodes_;
  const std::unordered_map<std::string, Course>* courseMap_;

 public:
  CourseGraph() : courseMap_(nullptr) {}

  // Build graph around a starting course
  void build(const std::unordered_map<std::string, Course>& courseMap, const std::string& startId) {
    courseMap_ = &courseMap;
    nodes_.clear();

    addPrerequisiteChain(startId);
    addDependentChain(startId);
  }

 private:
  // DFS backward (prerequisites)
  void addPrerequisiteChain(const std::string& id) {
    if (!courseMap_->count(id) || nodes_.count(id)) return;

    nodes_.insert(id);
    graph_.addNode(id);

    const Course& c = courseMap_->at(id);

    if (!c.prerequisite.empty()) {
      addPrerequisiteChain(c.prerequisite);
      graph_.addEdge(c.prerequisite, id);
    }
  }

  // BFS forward (dependents)
  void addDependentChain(const std::string& startId) {
    std::queue<std::string> q;
    q.push(startId);

    while (!q.empty()) {
      std::string curr = q.front();
      q.pop();

      for (const auto& [id, course] : *courseMap_) {
        if (course.prerequisite == curr && !nodes_.count(id)) {
          nodes_.insert(id);
          graph_.addNode(id);
          graph_.addEdge(curr, id);
          q.push(id);
        }
      }
    }
  }

 public:
  // Topological sort with priority
  std::vector<Course> topologicalSort() {
    std::unordered_map<std::string, int> inDeg;

    for (const auto& id : nodes_) {
      inDeg[id] = graph_.getInDegree(id);
    }

    auto cmp = [&](const std::string& a, const std::string& b) {
      return courseMap_->at(a).priority_score < courseMap_->at(b).priority_score;
    };

    std::priority_queue<std::string, std::vector<std::string>, decltype(cmp)> pq(cmp);

    for (const auto& [id, deg] : inDeg) {
      if (deg == 0) pq.push(id);
    }

    std::vector<Course> order;

    while (!pq.empty()) {
      std::string curr = pq.top();
      pq.pop();

      // 🔥 convert ID → Course here
      order.push_back(courseMap_->at(curr));

      for (const auto& neighbor : graph_.getNeighbors(curr)) {
        if (--inDeg[neighbor] == 0) {
          pq.push(neighbor);
        }
      }
    }

    return order;
  }

  // 🔍 Access actual course data when needed
  const Course& getCourse(const std::string& id) const { return courseMap_->at(id); }

  // Debug print
  void printGraph() const {
    for (const auto& id : nodes_) {
      const Course& c = courseMap_->at(id);

      std::cout << id << " (" << c.course_name << ")"
                << " | prereq: " << (c.prerequisite.empty() ? "none" : c.prerequisite)
                << " | unlocks: ";

      for (const auto& neighbor : graph_.getNeighbors(id)) {
        std::cout << neighbor << " ";
      }

      std::cout << "\n";
    }
  }
};