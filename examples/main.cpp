#include <complyer-sat.h>
#include <iostream>

using namespace std;

int main(){
  complyer_sat::expression e1(1), e2(2), e3(3);
  complyer_sat::expression eq = (e1 || e2 || !e3) && (!e1) && (e2) && (e3);
  complyer_sat::cnf c(eq);
  complyer_sat::solver s;
  auto m = s.solve(c);
  std::cout << (m.isSat() ? "sat" : "unsat") << std::endl;
  std::cout << m.getAssignment(1) << " " << m.getAssignment(2)  << " " << m.getAssignment(3) << std::endl;
}