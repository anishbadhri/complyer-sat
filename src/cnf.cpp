#include <complyer-sat.h>
#include <sstream>
#include <functional>


void complyer_sat::cnf::clause::addVariable(const expression &e) {
  if(e.getType() == expression::BASE or (e.getType() == expression::NOT and e.child(0).getType() == expression::BASE )){
    literals.push_back(e);
  } else {
    throw std::invalid_argument("Invalid literal. Must be BASE or NOT(BASE) only.");
  }
}

complyer_sat::expression complyer_sat::cnf::clause::operator[](int x) const {
  if(x < 0 || x >= literals.size()) throw std::invalid_argument("Out of bounds on clause index");
  return literals[x];
}

void complyer_sat::cnf::addClause(const clause &c) {
  if(c.size() == 0) return;
  for(expression i:c){
    variable_to_clause[i.getName()].insert(clauses.size());
  }
  clauses.push_back(c);
}

complyer_sat::cnf::clause complyer_sat::cnf::operator[](int x) const {
  if(x < 0 || x >= clauses.size()) throw std::invalid_argument("Out of bounds on cnf index");
  return clauses[x];
}

complyer_sat::cnf::cnf(const complyer_sat::expression& e) {
  auto expr = e.NNF();
  if(!isCNF(expr)){
    clauses.clear();
    tseytinTransformation(expr);
  }
}

bool complyer_sat::cnf::isCNF(const complyer_sat::expression& e) {
  std::function<bool(complyer_sat::expression, complyer_sat::cnf::clause &)> literal_traverse =
          [&](const complyer_sat::expression& cur, complyer_sat::cnf::clause &clause){
            if(cur.getType() != expression::NOT and cur.getType() != expression::BASE) return false;
            clause.addVariable(cur);
            return true;
          };
  std::function<bool(complyer_sat::expression, complyer_sat::cnf::clause &)> or_traverse =
          [&](const complyer_sat::expression& cur, complyer_sat::cnf::clause &clause){
            if(cur.getType() != expression::OR) return false;
            for(int i=0;i<2;i++){
              if(cur.child(i).getType() == expression::NOT or cur.child(i).getType() == expression::BASE){
                if(!literal_traverse(cur.child(i), clause)) return false;
              } else if(cur.child(i).getType() == expression::OR){
                if(!or_traverse(cur.child(i), clause)) return false;
              } else {
                return false;
              }
            }
            return true;
          };
  std::function<bool(complyer_sat::expression)> and_traverse =
          [&](const complyer_sat::expression& cur){
            if(cur.getType() != expression::AND) return false;
            for(int i=0;i<2;i++){
              complyer_sat::cnf::clause tmp;
              if(cur.child(i).getType() == expression::NOT or cur.child(i).getType() == expression::BASE) {
                if(!literal_traverse(cur.child(i), tmp)) return false;
              } else if(cur.child(i).getType() == expression::OR){
                if(!or_traverse(cur.child(i),tmp)) return false;
              } else if(cur.child(i).getType() == expression::AND){
                if(!and_traverse(cur.child(i))) return false;
              } else {
                throw std::logic_error("Unexpected operation during conversion");
              }
              addClause(tmp);
            }
            return true;
          };
  complyer_sat::cnf::clause tmp;
  if(e.getType() == expression::NOT or e.getType() == expression::BASE){
    if(!literal_traverse(e, tmp)) return false;
  } else if(e.getType() == expression::OR){
    if(!or_traverse(e,tmp)) return false;
  } else if(e.getType() == expression::AND){
    if(!and_traverse(e)) return false;
  } else {
    throw std::logic_error("Unexpected operation during conversion");
  }
  addClause(tmp);
  return true;
}

void complyer_sat::cnf::tseytinTransformation(const complyer_sat::expression &e) {
  std::string tseytin_variable_prefix = "Tseytin";
  int tseytin_count = 0;
  std::function<complyer_sat::expression(complyer_sat::expression)> recurse =
          [&](complyer_sat::expression cur){
            if(cur.getType() == expression::BASE) return cur;
            if(cur.getType() == expression::NOT){
              auto c = complyer_sat::expression(tseytin_variable_prefix + std::to_string(tseytin_count));
              tseytin_count++;
              auto a = recurse(cur.child(0));
              complyer_sat::cnf::clause cc;
              cc.addVariable(!a); cc.addVariable(!c);
              addClause(cc);
              cc = complyer_sat::cnf::clause();
              cc.addVariable(a); cc.addVariable(c);
              addClause(cc);
              return c;
            }
            auto a = recurse(cur.child(0));
            auto b = recurse(cur.child(1));
            auto c = complyer_sat::expression(tseytin_variable_prefix + std::to_string(tseytin_count));
            tseytin_count++;
            if(cur.getType() == expression::AND){
              complyer_sat::cnf::clause cc;
              cc.addVariable(!a); cc.addVariable(!b); cc.addVariable(c);
              addClause(cc);
              cc = complyer_sat::cnf::clause();
              cc.addVariable(a); cc.addVariable(!c);
              addClause(cc);
              cc = complyer_sat::cnf::clause();
              cc.addVariable(b); cc.addVariable(!c);
              addClause(cc);
            } else if(cur.getType() == expression::OR){
              complyer_sat::cnf::clause cc;
              cc.addVariable(a); cc.addVariable(b); cc.addVariable(!c);
              addClause(cc);
              cc = complyer_sat::cnf::clause();
              cc.addVariable(!a); cc.addVariable(c);
              addClause(cc);
              cc = complyer_sat::cnf::clause();
              cc.addVariable(!b); cc.addVariable(c);
              addClause(cc);
            } else {
              throw std::logic_error("Unexpected operation during conversion");
            }
            return c;
          };
  complyer_sat::cnf::clause cc;
  cc.addVariable(recurse(e));
  addClause(cc);
}
