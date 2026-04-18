// #pragma once
// // ============================================================
// // course_graph.h
// // Builds a directed prerequisite graph from a starting course.
// // Depends on directed_graph.h which provides:
// //   addNode(node)
// //   addEdge(from, to)
// //   getNeighbors(node)  -> vector of nodes reachable FROM node
// //   getInDegree(node)   -> number of incoming edges
// //   hasNode(node)       -> bool, O(1)
// // ============================================================
 
// #include "directed_graph.h"
// #include <string>
// #include <vector>
// #include <unordered_map>
// #include <unordered_set>
// #include <queue>
// #include <functional>
 
// // ── Course struct ─────────────────────────────────────────────────────────────
 
// struct Course {
//     std::string course_id;
//     std::string course_name;
//     int         units;
//     std::string prerequisite;   // single direct prerequisite (empty if none)
//     std::string major;
//     std::string division;
//     double      priority_score;
//     double      difficulty_score;
//     bool        junior_plus_standing;
//     bool        taken;
// };
 
// // ── CourseGraph ───────────────────────────────────────────────────────────────
// // Wraps directed_graph and adds course-specific logic.
// // Edge direction: prerequisite → course
// //   (meaning: take the prereq before you can take the course)
 
// class CourseGraph {
// public:
 
//     // ── Build graph from a starting course ───────────────────────────────────
//     // Performs a BFS/DFS from startId, following the prerequisite chain
//     // forward (i.e. what courses does this unlock?) using the courseMap.
//     // Also walks backward to pull in all prerequisites of startId.
//     //
//     // courseMap: all courses loaded from the database, keyed by course_id
//     // startId:   the course to start building from (e.g. "CS010A")
 
//     void build(const std::unordered_map<std::string, Course>& courseMap,
//                const std::string& startId)
//     {
//         courseMap_ = &courseMap;
 
//         if (!courseMap.count(startId)) {
//             std::cerr << "Starting course not found: " << startId << "\n";
//             return;
//         }
 
//         // Step 1: Walk BACKWARD from startId to add all its prerequisites
//         //         so the graph is complete going into startId
//         addPrerequisiteChain(startId);
 
//         // Step 2: Walk FORWARD from startId via BFS
//         //         adding every course that startId (transitively) unlocks
//         addDependentChain(startId);
//     }
 
//     // ── Topological sort (Kahn's algorithm) ──────────────────────────────────
//     // Returns courses in valid take-order (prerequisites before dependents).
//     // Ties broken by priority_score descending.
 
//     std::vector<std::string> topologicalSort() const {
//         // Recalculate in-degrees for nodes in our graph
//         std::unordered_map<std::string, int> inDeg;
//         for (const auto& id : nodes_) {
//             inDeg[id] = graph_.getInDegree(id);
//         }
 
//         // Priority queue: highest priority_score first
//         auto cmp = [&](const std::string& a, const std::string& b) {
//             double pa = courseMap_->count(a) ? courseMap_->at(a).priority_score : 0.0;
//             double pb = courseMap_->count(b) ? courseMap_->at(b).priority_score : 0.0;
//             return pa < pb; // max-heap
//         };
//         std::priority_queue<std::string,
//                             std::vector<std::string>,
//                             decltype(cmp)> pq(cmp);
 
//         for (const auto& [id, deg] : inDeg) {
//             if (deg == 0) pq.push(id);
//         }
 
//         std::vector<std::string> order;
//         while (!pq.empty()) {
//             std::string curr = pq.top(); pq.pop();
//             order.push_back(curr);
 
//             for (const auto& neighbor : graph_.getNeighbors(curr)) {
//                 if (!inDeg.count(neighbor)) continue;
//                 inDeg[neighbor]--;
//                 if (inDeg[neighbor] == 0) pq.push(neighbor);
//             }
//         }
 
//         return order;
//     }
 
//     // ── Accessors ─────────────────────────────────────────────────────────────
 
//     // All course IDs in the graph
//     const std::unordered_set<std::string>& getNodes() const { return nodes_; }
 
//     // Check if a course is in the graph
//     bool hasNode(const std::string& id) const { return graph_.hasNode(id); }
 
//     // Get courses that this course directly unlocks
//     std::vector<std::string> getUnlocks(const std::string& id) const {
//         return graph_.getNeighbors(id);
//     }
 
//     // Get direct prerequisite of a course (empty string if none)
//     std::string getPrerequisite(const std::string& id) const {
//         if (!courseMap_->count(id)) return "";
//         return courseMap_->at(id).prerequisite;
//     }
 
//     // Get all courses with no prerequisites in the graph (entry points)
//     std::vector<std::string> getRoots() const {
//         std::vector<std::string> roots;
//         for (const auto& id : nodes_) {
//             if (graph_.getInDegree(id) == 0) roots.push_back(id);
//         }
//         return roots;
//     }
 
//     // Get all courses with no dependents in the graph (terminal courses)
//     std::vector<std::string> getLeaves() const {
//         std::vector<std::string> leaves;
//         for (const auto& id : nodes_) {
//             if (graph_.getNeighbors(id).empty()) leaves.push_back(id);
//         }
//         return leaves;
//     }
 
//     // Print the graph as an adjacency list for debugging
//     void printGraph() const {
//         std::cout << "\n── Course Prerequisite Graph ──\n";
//         std::cout << "  Nodes: " << nodes_.size() << "\n";
 
//         // Print in topological order for readability
//         auto order = topologicalSort();
//         for (const auto& id : order) {
//             std::string prereq = getPrerequisite(id);
//             auto unlocks = graph_.getNeighbors(id);
 
//             std::cout << "  " << std::left << std::setw(12) << id;
//             if (!prereq.empty())
//                 std::cout << "  prereq: " << std::setw(12) << prereq;
//             else
//                 std::cout << "  prereq: " << std::setw(12) << "(none)";
 
//             if (!unlocks.empty()) {
//                 std::cout << "  unlocks: ";
//                 for (int i = 0; i < (int)unlocks.size(); i++) {
//                     std::cout << unlocks[i];
//                     if (i < (int)unlocks.size() - 1) std::cout << ", ";
//                 }
//             }
//             std::cout << "\n";
//         }
//     }
 
// private:
 
//     DirectedGraph                               graph_;
//     std::unordered_set<std::string>             nodes_;
//     const std::unordered_map<std::string, Course>* courseMap_ = nullptr;
 
//     // Walk BACKWARD: add startId and all its transitive prerequisites
//     void addPrerequisiteChain(const std::string& id) {
//         if (!courseMap_->count(id)) return;
//         if (nodes_.count(id)) return; // already visited
 
//         // Add this node
//         graph_.addNode(id);
//         nodes_.insert(id);
 
//         const Course& c = courseMap_->at(id);
 
//         // Recurse into prerequisite
//         if (!c.prerequisite.empty() && courseMap_->count(c.prerequisite)) {
//             addPrerequisiteChain(c.prerequisite);
 
//             // Edge: prerequisite → this course
//             graph_.addEdge(c.prerequisite, id);
//         }
//     }
 
//     // Walk FORWARD: BFS from startId, adding everything startId unlocks
//     void addDependentChain(const std::string& startId) {
//         std::queue<std::string> bfsQueue;
//         bfsQueue.push(startId);
 
//         while (!bfsQueue.empty()) {
//             std::string curr = bfsQueue.front(); bfsQueue.pop();
 
//             // Find all courses whose prerequisite is curr
//             for (const auto& [id, course] : *courseMap_) {
//                 if (course.prerequisite == curr) {
//                     if (!nodes_.count(id)) {
//                         graph_.addNode(id);
//                         nodes_.insert(id);
//                         bfsQueue.push(id);
//                     }
//                     // Add edge even if node already existed
//                     // (addEdge should be idempotent in directed_graph.h)
//                     graph_.addEdge(curr, id);
//                 }
//             }
//         }
//     }
// };