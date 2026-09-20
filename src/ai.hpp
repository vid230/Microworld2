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

  bool FrontIsWall(const Percepts& percepts) const;
  bool RayHasTreasure(const std::vector<std::string>& ray) const;
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



