#include <complyer-sat.h>


complyer_sat::expression complyer_sat::l_and(const expression &e1, const expression &e2) {
  return complyer_sat::expression(expression::AND, e1, e2);
}

complyer_sat::expression complyer_sat::l_or(const expression &e1, const expression &e2) {
  return complyer_sat::expression(expression::OR, e1, e2);
}

complyer_sat::expression complyer_sat::l_not(const expression &e) {
  return complyer_sat::expression(expression::NOT, e);
}

complyer_sat::expression complyer_sat::implies(const expression &e1, const expression &e2) {
  return complyer_sat::expression(expression::OR, l_not(e1), e2);
}

complyer_sat::expression & complyer_sat::expression::child(int c) const {
  if(auto children = std::get_if<non_base_operands>(&operands)){
    if(c < 0 || c > children->size()) throw std::invalid_argument("Out of Bounds");
    return *(*children)[c];
  }
  throw std::invalid_argument("No child for base class");
}

std::string complyer_sat::expression::getName() const {
  switch (getType()) {
    case NOT:
      return "(!" + child(0).getName() + ")";
    case AND:
      return "(" + child(0).getName() + " & " + child(1).getName() + ")";
    case OR:
      return "(" + child(0).getName() + " | " + child(1).getName() + ")";
    case BASE:
      return std::get<base_operands>(operands);
  }
}

complyer_sat::expression::expression(int base) {
  if(base <= 0) throw std::invalid_argument("Positive integers of base");
  type = BASE;
  operands = std::to_string(base);
}

complyer_sat::expression::expression(complyer_sat::expression::Operations type, const complyer_sat::expression &e1,
                                     const complyer_sat::expression &e2) {
  if(type == BASE or type == NOT) throw std::invalid_argument("Call different constructor for base");
  this->type = type;
  std::get<non_base_operands>(operands).push_back(std::make_shared<expression>(e1));
  std::get<non_base_operands>(operands).push_back(std::make_shared<expression>(e2));
}

complyer_sat::expression::expression(complyer_sat::expression::Operations type, const complyer_sat::expression &e) {
  if(type != NOT) throw std::invalid_argument("Call different constructor for base");
  this->type = type;
  std::get<non_base_operands>(operands).push_back(std::make_shared<expression>(e));
}

complyer_sat::expression complyer_sat::expression::NNF(bool negate) const {
  if(!negate) {
    switch(getType()) {
      case NOT: return child(0).NNF(true);
      case AND:
      case OR:
        return expression(type, child(0).NNF(false), child(1).NNF(false));
      case BASE:
        return *this;
    }
  } else {
    switch (getType()) {
      case NOT: return child(0).NNF(false);
      case AND:
        return expression(OR, child(0).NNF(true), child(1).NNF(true));
      case OR:
        return expression(AND, child(0).NNF(true), child(1).NNF(true));
      case BASE:
        return expression(NOT, *this);
    }
  }
}

complyer_sat::expression::expression(complyer_sat::base_operands base) {
  type = BASE;
  operands = base;
}
