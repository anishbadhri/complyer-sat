#include <complyer-sat.h>
#include <functional>
#include <stack>
#include <iostream>
#include <queue>

complyer_sat::model complyer_sat::solver::solve(complyer_sat::cnf c) {
  two_sat ts;
  horn_sat hs;
  dpll ds;
  if(ts.check(c)){
    return ts.solve(c);
  } else if(hs.check(c)){
    return hs.solve(c);
  }
  return ds.solve(c);
}

bool complyer_sat::solver::two_sat::check(const complyer_sat::cnf &c) {
  for(auto i:c){
    if(i.size() > 2) return false;
  }
  return sub_solvers::check(c);
}

complyer_sat::model complyer_sat::solver::two_sat::solve(const complyer_sat::cnf &c) {
  model result(c);
  std::map<std::string,int> scc;
  std::map<std::string, std::vector<std::string>> forward_graph, reverse_graph;

  for(const auto &i:c){
    std::string src, dest;
    if(i.size() > 2) throw std::logic_error("Clause size greater than 2");
    if(i.size() == 0) continue;
    else if(i.size() == 1){
      src = l_not(i[0]).NNF().getName();
      dest = i[0].NNF().getName();
      forward_graph[src].push_back(dest);
      reverse_graph[dest].push_back(src);
    } else {
      src = l_not(i[0]).NNF().getName();
      dest = i[1].NNF().getName();
      forward_graph[src].push_back(dest);
      reverse_graph[dest].push_back(src);
      src = l_not(i[1]).NNF().getName();
      dest = i[0].NNF().getName();
      forward_graph[src].push_back(dest);
      reverse_graph[dest].push_back(src);
    }
  }

  std::set<std::string> visited;
  std::stack<std::string> topological_sort;
  int scc_count = 0;

  std::function<void(std::string)> forward_dfs = [&](std::string cur){
    if(visited.find(cur) != visited.end())
      return;
    visited.insert(cur);
    for(std::string child:forward_graph[cur]){
      forward_dfs(child);
    }
    topological_sort.push(cur);
  };

  std::function<void(std::string)> reverse_dfs = [&](std::string cur) {
    if(visited.find(cur) != visited.end())
      return;
    visited.insert(cur);
    for(std::string child:reverse_graph[cur]){
      reverse_dfs(child);
    }
    scc[cur] = scc_count;
  };

  for(auto i:result.unique_vars){
    forward_dfs(i);
    complyer_sat::expression tmp(std::stoi(i));
    std::string not_i = l_not(tmp).NNF().getName();
    forward_dfs(not_i);
  }

  std::stack<std::string> topo_copy = topological_sort;
  visited.clear();

  while(!topo_copy.empty()){
    std::string x = topo_copy.top();
    topo_copy.pop();
    if(visited.find(x) == visited.end()){
      reverse_dfs(x);
      scc_count++;
    }
  }

  visited.clear();

  for(auto i:result.unique_vars){
    complyer_sat::expression tmp(std::stoi(i));
    std::string not_i = l_not(tmp).NNF().getName();
    if(scc[i] == scc[not_i]){
      result.sat = model::UNSAT;
      return result;
    }
  }
  result.sat = model::SAT;
  while(!topological_sort.empty()){
    std::string cur = topological_sort.top();
    topological_sort.pop();
    std::string variable = cur;
    bool not_set = false;
    if(cur.substr(0,2) == "(!"){
      variable = cur.substr(2, cur.length() - 3);
      not_set = true;
    }
    if(result.assignments[variable] != model::UNKNOWN ) continue;
    result.setAssignment(variable, not_set ? model::FALSE : model::TRUE);
  }
  return result;
}

bool complyer_sat::solver::horn_sat::check(const complyer_sat::cnf &c) {
  for(auto clause:c){
    int positive_count = 0;
    for(auto variable:clause){
      if(variable.getType() == expression::BASE) positive_count++;
    }
    if(positive_count > 1)
      return false;
  }
  return sub_solvers::check(c);
}

complyer_sat::model complyer_sat::solver::horn_sat::solve(const complyer_sat::cnf &c) {
  struct clause_score {
    int score;
    std::string enable;
    bool False_Termination;
  };
  clause_score score[c.size()];
  std::map<std::string, std::set<int>> clause_index;
  std::queue<int> satisfied_clauses;
  for(int i=0;i<c.size();i++){
    if(c[i].size() == 0) continue;
    score[i].False_Termination = true;
    score[i].score = 0;
    for(auto j:c[i]){
      if(j.getType() == expression::NOT){
        clause_index[l_not(j).NNF().getName()].insert(i);
        score[i].score++;
      } else {
        score[i].False_Termination = false;
        score[i].enable = j.getName();
      }
    }
    if(score[i].score == 0){
      satisfied_clauses.push(i);
    }
  }

  model result(c);
  for(std::string var: result.unique_vars){
    result.setAssignment(var,model::FALSE);
  }

  while(!satisfied_clauses.empty()){
    int cur = satisfied_clauses.front();
    satisfied_clauses.pop();
    if(score[cur].False_Termination){
      model tmp(c);
      tmp.sat = model::UNSAT;
      return tmp;
    }
    std::string enable_string = score[cur].enable;
    if(result.assignments[enable_string] == model::TRUE) continue;
    result.setAssignment(enable_string, model::TRUE);
    for(int i:clause_index[enable_string]) {
      score[i].score--;
      if(score[i].score == 0){
        satisfied_clauses.push(i);
      }
    }
  }

  result.sat = model::SAT;
  return result;
}

complyer_sat::model complyer_sat::solver::dpll::solve(const complyer_sat::cnf &c) {
  model result(c);

  for(long long i=0;i < (1 << result.unique_vars.size());i++){
    int enable = i;
    for(auto j:result.unique_vars){
      if(enable % 2) result.setAssignment(j, model::TRUE);
      else result.setAssignment(j, model::FALSE);
      enable >>= 1;
    }
    int enabled_count = 0;
    for(auto j:c){
      bool clause_enabled = false;
      for(auto k:j){
        if(k.getType() == expression::NOT){
          if(result.assignments[k.child(0).getName()] == model::FALSE)
            clause_enabled = true;
        }
        else {
          if(result.assignments[k.getName()] == model::TRUE)
            clause_enabled = true;
        }
      }
      enabled_count += clause_enabled;
    }
    if(enabled_count == c.size()){
      result.sat = model::SAT;
      return result;
    }
  }
  result = model(c);
  result.sat = model::UNSAT;
  return result;
}
