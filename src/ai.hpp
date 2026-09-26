#pragma once

#include<algorithm>
#include<string>
#include<random>
#include<map>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include"percepts.hpp"
#include"comm.hpp"
#include <queue>
#include <set>

class AI {
protected:
  // Necessary, do not delete.
  unsigned id;
  unsigned agent_speed;
  std::mt19937_64* rng;
  Symbols symbols;
  Costs costs;

  struct Point
  {
    int x;
    int y;

    Point(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator<(const Point& other) const
    {
      if (x != other.x)
      {
        return x < other.x;
      }

      return y < other.y;
    }

    bool operator==(const Point& other) const
    {
      return ((x == other.x) && (y == other.y));
    }
  };
  
  int trap_scan_step;

  bool initialized;
  Point pos;
  int dir;
  std::string last_cmd;

  std::map<Point, std::string> known_map;

  bool FrontIsWall(const Percepts& percepts) const;
  bool RayHasTreasure(const std::vector<std::string>& ray) const;

  Point DirVec(int d) const;
  Point Add(Point a, Point b) const;
  Point Mul(Point a, int k) const;

  void ApplyLastCommand();
  void RememberCell(Point p, const std::string& cell);
  void IntegrateRay(Point start, Point step, const std::vector<std::string>& ray);
  void IntegratePercepts(const Percepts& percepts);
  void SaveIssuedCommand(const std::string& cmd);
public:
  AI();
  AI(
     unsigned id, 
     unsigned agent_speed,
     std::mt19937_64* rng,
     Symbols symbols,
     Costs costs);
  void PrintPercepts(const Percepts & percepts);
  std::vector<std::string> Run(
			       Percepts & percepts,
			       AgentComm * comms);
};



