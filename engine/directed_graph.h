#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename T>
class DirectedGraph {
 private:
  // Adjacency list: node -> list of neighbors
  std::unordered_map<T, std::vector<T>> adj;

  // In-degree count: node -> number of incoming edges
  std::unordered_map<T, int> inDegree;

 public:
  // Add a node to the graph
  void addNode(const T& node) {
    // Only add if it doesn't already exist
    if (adj.find(node) == adj.end()) {
      adj[node] = std::vector<T>();
      inDegree[node] = 0;
    }
  }

  // Add a directed edge from 'from' -> 'to'
  void addEdge(const T& from, const T& to) {
    // Ensure both nodes exist
    addNode(from);
    addNode(to);

    adj[from].push_back(to);
    inDegree[to]++;
  }

  // Get neighbors of a node
  const std::vector<T>& getNeighbors(const T& node) const {
    auto it = adj.find(node);
    if (it == adj.end()) {
      throw std::out_of_range("Node does not exist");
    }
    return it->second;
  }

  // Get in-degree of a node
  int getInDegree(const T& node) const {
    auto it = inDegree.find(node);
    if (it == inDegree.end()) {
      throw std::out_of_range("Node does not exist");
    }
    return it->second;
  }

  // Get all nodes
  std::vector<T> getNodes() const {
    std::vector<T> nodes;
    for (const auto& pair : adj) {
      nodes.push_back(pair.first);
    }
    return nodes;
  }

  // Check if node exists
  bool hasNode(const T& node) const { return adj.find(node) != adj.end(); }

  // Optional: total number of nodes
  size_t size() const { return adj.size(); }
};

#endif