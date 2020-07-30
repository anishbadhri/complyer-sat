#include <complyer-sat.h>

complyer_sat::model::states complyer_sat::model::getAssignment(int x) {
  std::string cur = std::to_string(x);
  if(assignments.find(cur) == assignments.end()) throw std::invalid_argument("Invalid argument");
  return assignments[cur];
}

std::vector<std::pair<complyer_sat::base_operands, complyer_sat::model::states>>
complyer_sat::model::setAssignment(complyer_sat::base_operands s, complyer_sat::model::states st) {
  std::vector<std::pair<complyer_sat::base_operands, states>> x;
  assignments[s] = st;
  x.push_back({s, st});
  // TODO(anishbadhri): Implement Unit Propagation
  return x;
}

complyer_sat::model::model(const complyer_sat::cnf &c) {
  for(auto i:c){
    for(auto j:i){
      if(j.getType() == expression::NOT) {
        assignments[j.child(0).getName()] = UNKNOWN;
        unique_vars.insert(j.child(0).getName());
      }
      else {
        assignments[j.getName()] = UNKNOWN;
        unique_vars.insert(j.getName());
      }
    }
  }
  sat = UNSAT;
}
