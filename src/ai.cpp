#include"ai.hpp"

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI() : trap_scan_step(0), initialized(false), pos(0, 0), dir(0), last_cmd(""), has_last_disarm_target(false), last_disarm_target(0, 0) {}
AI::AI(
    unsigned id, 
    unsigned agent_speed,
    std::mt19937_64 * rng,
    Symbols symbols,
    Costs costs
)
  : id(id), agent_speed(agent_speed), rng(rng),
    symbols(symbols), costs(costs), trap_scan_step(0),
    initialized(false), pos(0, 0), dir(0), last_cmd(""),
    has_last_disarm_target(false), last_disarm_target(0, 0)
{}

void AI::PrintPercepts(const Percepts & percepts) {
  std::cout << "DISTANCE: " << percepts.detector << std::endl;
  std::cout << "CURRENT:  " << percepts.current[0] << std::endl;
  std::cout << "FORWARD:  ";
  for(std::vector<std::string>::const_iterator it = percepts.forward.begin();
      it != percepts.forward.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "LEFT:     ";
  for(std::vector<std::string>::const_iterator it = percepts.left.begin();
      it != percepts.left.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "BACKWARD: ";
  for(std::vector<std::string>::const_iterator it = percepts.backward.begin();
      it != percepts.backward.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "RIGHT:    ";
  for(std::vector<std::string>::const_iterator it = percepts.right.begin();
      it != percepts.right.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "Others:\n";
  for(size_t i = 0; i < percepts.others.size(); i++) {
    std::cout << "   " << i << ": " << percepts.others[i].to_string() << std::endl;
  }
}

bool AI::FrontIsWall(const Percepts& percepts) const
{
  if (percepts.forward.empty())
  {
    return false;
  }

  return percepts.forward[0] == symbols.wall;
}

bool AI::RayHasTreasure(const std::vector<std::string>& ray) const
{
  for (const std::string& cell : ray)
  {
    if (cell == symbols.treasure)
    {
      return true;
    }

    if (cell == symbols.wall)
    {
      return false;
    }
  }

  return false;
}

AI::Point AI::DirVec(int d) const
{
  int r = ((d % 4) + 4) % 4;

  if (r == 0)
  {
    return Point(0, -1);
  }

  if (r == 1)
  {
    return Point(1, 0);
  }

  if (r == 2)
  {
    return Point(0, 1);
  }

  return Point(-1, 0);
}

AI::Point AI::Add(Point a, Point b) const
{
  return Point(a.x + b.x, a.y + b.y);
}

AI::Point AI::Mul(Point a, int k) const
{
  return Point(a.x * k, a.y * k);
}

void AI::ApplyLastCommand()
{
  if (last_cmd == "F")
  {
    pos = Add(pos, DirVec(dir));
  }
  else if (last_cmd == "B")
  {
    pos = Add(pos, Mul(DirVec(dir), -1));
  }
  else if (last_cmd == "L")
  {
    dir = (dir + 3) % 4;
  }
  else if (last_cmd == "R")
  {
    dir = (dir + 1) % 4;
  }
}

void AI::RememberCell(Point p, const std::string& cell)
{
  known_map[p] = cell;

  if (IsDefinitelySafeCell(cell))
  {
    MarkSafe(p);
  }
}

void AI::IntegrateRay(Point start, Point step, const std::vector<std::string>& ray)
{
  for (size_t i = 0; i < ray.size(); i++)
  {
    Point p = Add(start, Mul(step, static_cast<int>(i)));
    RememberCell(p, ray[i]);
  }
}

void AI::IntegratePercepts(const Percepts& percepts)
{
  if (!percepts.current.empty())
  {
    RememberCell(pos, percepts.current[0]);
  }

  Point forward = DirVec(dir);
  Point right = DirVec(dir + 1);
  Point backward = DirVec(dir + 2);
  Point left = DirVec(dir + 3);

  IntegrateRay(Add(pos, forward), forward, percepts.forward);
  IntegrateRay(Add(pos, right), right, percepts.right);
  IntegrateRay(Add(pos, backward), backward, percepts.backward);
  IntegrateRay(Add(pos, left), left, percepts.left);
}

bool AI::IsKnownPassable(Point p) const
{
  std::map<Point, std::string>::const_iterator it = known_map.find(p);

  if (it == known_map.end())
  {
    return false;
  }

  return it->second != symbols.wall;
}

bool AI::IsTrapSuspect(Point p) const
{
  return trap_score.find(p) != trap_score.end();
}

bool AI::IsKnownSafePassable(Point p) const
{
  std::map<Point, std::string>::const_iterator it = known_map.find(p);

  if (it == known_map.end())
  {
    return false;
  }

  if (it->second == symbols.wall)
  {
    return false;
  }

  if ((it->second == symbols.treasure) || (it->second == symbols.disarmed_mine) || (it->second == symbols.exploded_mine))
  {
    return true;
  }

  if (safe_cells.find(p) != safe_cells.end())
  {
    return true;
  }

  if (IsTrapSuspect(p))
  {
    return false;
  }

  // If we know it and it is not currently suspicious, allow it.
  return true;
}

bool AI::IsKnownTreasure(Point p) const
{
  std::map<Point, std::string>::const_iterator it = known_map.find(p);

  if (it == known_map.end())
  {
    return false;
  }

  return it->second == symbols.treasure;
}

std::string AI::CommandTowardDirection(int target_dir) const
{
  target_dir = ((target_dir % 4) + 4) % 4;

  if (target_dir == dir)
  {
    return "F";
  }

  if (target_dir == (dir + 1) % 4)
  {
    return "R";
  }

  if (target_dir == (dir + 3) % 4)
  {
    return "L";
  }

  return "B";
}

std::string AI::FirstStepCommand(Point next) const
{
  Point delta(next.x - pos.x, next.y - pos.y);

  for (int d = 0; d < 4; d++)
  {
    if (DirVec(d) == delta)
    {
      return CommandTowardDirection(d);
    }
  }

  return "R";
}

bool AI::FindPath(Point target, std::vector<Point>& path) const
{
  std::queue<Point> q;
  std::set<Point> visited;
  std::map<Point, Point> parent;

  q.push(pos);
  visited.insert(pos);

  while (!q.empty())
  {
    Point current = q.front();
    q.pop();

    if (current == target)
    {
      path.clear();

      Point p = target;

      while (!(p == pos))
      {
        path.push_back(p);
        p = parent[p];
      }

      std::reverse(path.begin(), path.end());
      return true;
    }

    for (int d = 0; d < 4; d++)
    {
      Point next = Add(current, DirVec(d));

      if (visited.find(next) != visited.end())
      {
        continue;
      }

      if (!IsKnownSafePassable(next))
      {
        continue;
      }

      visited.insert(next);
      parent[next] = current;
      q.push(next);
    }
  }

  return false;
}

bool AI::FindNearestKnownTreasure(std::vector<Point>& path) const
{
  bool found = false;
  std::vector<Point> best_path;

  for (std::map<Point, std::string>::const_iterator it = known_map.begin(); it != known_map.end(); ++it)
  {
    if (it->second != symbols.treasure)
    {
      continue;
    }

    std::vector<Point> candidate_path;

    if (FindPath(it->first, candidate_path))
    {
      if (!found || candidate_path.size() < best_path.size())
      {
        found = true;
        best_path = candidate_path;
      }
    }
  }

  if (found)
  {
    path = best_path;
  }

  return found;
}

int AI::Manhattan(Point a, Point b) const
{
  int dx = a.x - b.x;
  int dy = a.y - b.y;

  if (dx < 0)
  {
    dx = -dx;
  }

  if (dy < 0)
  { 
    dy = -dy;
  }

  return dx + dy;
}

bool AI::IsDefinitelySafeCell(const std::string& cell) const
{
  return ((cell == symbols.wall) || (cell == symbols.treasure) || (cell == symbols.disarmed_mine) || (cell == symbols.exploded_mine));
}

void AI::MarkSafe(Point p)
{
  safe_cells.insert(p);
  trap_score.erase(p);
}

void AI::ResolveLastDisarmResult()
{
  if (!has_last_disarm_target)
  {
    return;
  }

  std::map<Point, std::string>::const_iterator it = known_map.find(last_disarm_target);

  if (it != known_map.end())
  {
    // If it became disarmed/exploded, RememberCell already marks it safe.
    // If it is still open-looking, then my D command probably failed,
    // so the target cell is probably not a trap.
    if ((it->second != symbols.disarmed_mine) && (it->second != symbols.exploded_mine))
    {
      MarkSafe(last_disarm_target);
    }
  }

  has_last_disarm_target = false;
}

void AI::AnalyzeTrapDetector(const Percepts& percepts)
{
  // Current cell is always safe, because the rogue is alive.
  MarkSafe(pos);

  // If there are no traps left, every known non-wall cell is safe.
  if (percepts.detector == -1)
  {
    trap_score.clear();

    for (std::map<Point, std::string>::const_iterator it = known_map.begin(); it != known_map.end(); ++it)
    {
      if (it->second != symbols.wall)
      {
        MarkSafe(it->first);
      }
    }

    return;
  }

  int d = percepts.detector;

  // Any known cell closer than the nearest trap distance cannot be a trap.
  for (std::map<Point, std::string>::const_iterator it = known_map.begin(); it != known_map.end(); ++it)
  {
    Point p = it->first;
    const std::string& cell = it->second;

    if (cell == symbols.wall)
    {
      MarkSafe(p);
      continue;
    }

    if (Manhattan(pos, p) < d)
    {
      MarkSafe(p);
    }
  }

  // Cells exactly at detector distance are suspicious.
  // We only score known non-wall cells.
  for (std::map<Point, std::string>::const_iterator it = known_map.begin(); it != known_map.end(); ++it)
  {
    Point p = it->first;
    const std::string& cell = it->second;

    if (cell == symbols.wall)
    {
      continue;
    }

    if (safe_cells.find(p) != safe_cells.end())
    {
      continue;
    }

    if (Manhattan(pos, p) == d)
    {
      trap_score[p]++;
    }
  }
}

void AI::SaveIssuedCommand(const std::string& cmd)
{
  last_cmd = cmd;
}

std::vector<std::string> AI::Run(Percepts & percepts, AgentComm * comms)
{
  if (!initialized)
  {
    initialized = true;
  }
  else
  {
    ApplyLastCommand();
  }

  IntegratePercepts(percepts);

  auto issue = [&](const std::string& cmd)
  {
    SaveIssuedCommand(cmd);
    return std::vector<std::string>{cmd};
  };

  // 1. If standing on treasure, take it.
  if (!percepts.current.empty() && percepts.current[0] == symbols.treasure)
  {
    return issue("T");
  }

  // 2. If a trap is adjacent, scan/disarm around us. detector == 1 means the nearest trap is one cell away.
  if (percepts.detector == 1)
  {
    bool front_is_wall = FrontIsWall(percepts);

    if (trap_scan_step % 2 == 0 && !front_is_wall)
    {
      trap_scan_step++;
      return issue("D");
    }
    else
    {
      trap_scan_step++;
      return issue("R");
    }
  }
  else
  {
    trap_scan_step = 0;
  }

  std::vector<Point> treasure_path;

  if (FindNearestKnownTreasure(treasure_path) && !treasure_path.empty())
  {
    std::string cmd = FirstStepCommand(treasure_path[0]);
    return issue(cmd);
  }

  // 3. If we see treasure, move toward it.
  if (RayHasTreasure(percepts.forward))
  {
    if (!FrontIsWall(percepts))
    {
      return issue("F");
    }
  }

  if (RayHasTreasure(percepts.left))
  {
    return issue("L");
  }

  if (RayHasTreasure(percepts.right))
  {
    return issue("R");
  }

  if (RayHasTreasure(percepts.backward))
  {
    return issue("R");
  }

  // 4. Basic exploration: move forward if possible.
  if (!FrontIsWall(percepts))
  {
    return issue("F");
  }

  // 5. If blocked, turn randomly left or right.
  if ((*rng)() % 2 == 0)
  {
    return issue("L");
  }
  else
  {
    return issue("R");
  }
}



