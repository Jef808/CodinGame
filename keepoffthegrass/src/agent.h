#ifndef AGENT_H_
#define AGENT_H_

#include <string>
#include <vector>

#include "worldinfo.h"

class Agent {
  public:
  Agent() = default;

  void choose_actions(const WorldInfo& worldinfo);

  void output_actions() const;

  private:
  std::vector<std::string> m_actions;
};


#endif // AGENT_H_
