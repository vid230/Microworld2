#include"ai.hpp"

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI() : trap_scan_step(0) {}
AI::AI(
    unsigned id, 
    unsigned agent_speed,
    std::mt19937_64 * rng,
    Symbols symbols,
    Costs costs
)
  : id(id), agent_speed(agent_speed), rng(rng),
    symbols(symbols), costs(costs), trap_scan_step(0)
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

std::vector<std::string> AI::Run(
    Percepts & percepts,
    AgentComm * comms
) {
  std::cout << "------------------------------------------------\n";
  std::cout << "AGENT ID: " << id << std::endl;
  PrintPercepts(percepts);
  std::vector<std::string> cmds {"R", "B", "L", "F", "U", "D"};
  std::shuffle(cmds.begin(), cmds.end(), *rng);
  std::cout << "CMD:      " << cmds[0] << std::endl;
  return {cmds[0]};
}



