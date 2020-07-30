#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <map>
#include <set>

namespace complyer_sat {
  class expression;
  class cnf;
  class model;
  class solver;

  namespace {
    typedef std::vector<std::shared_ptr<expression>> non_base_operands;
    typedef std::string base_operands;
  }

  complyer_sat::expression l_and(const expression &e1, const expression &e2);
  complyer_sat::expression l_or(const expression &e1, const expression &e2);
  complyer_sat::expression l_not(const expression &e);
  complyer_sat::expression implies(const expression &e1, const expression &e2);

  class expression {
    public:
      enum Operations {NOT, AND, OR, BASE};

      explicit expression(int base);
      expression(Operations type, const expression &e);
      expression(Operations type, const expression &e1, const expression &e2);

      [[nodiscard]] complyer_sat::expression & child(int c) const;
      [[nodiscard]] inline Operations getType() const{ return type; }
      std::string getName() const;

      [[nodiscard]] expression NNF() const {return NNF(false);}

      inline expression operator!() const {return l_not(*this);}
      inline expression operator&&(const expression &e) const {return l_and(*this, e);}
      inline expression operator||(const expression &e) const {return l_or(*this, e);}
    private:


      explicit expression(base_operands base);
      Operations type;
      [[nodiscard]] expression NNF(bool) const;

      std::variant<non_base_operands, base_operands> operands;
      friend class complyer_sat::cnf;
  };

  class cnf {
    public:

      cnf() = default;
      explicit cnf(const expression& e);
      class clause {
        public:
          void addVariable(const expression &e);

          expression operator[](int x) const;
          [[nodiscard]] inline auto size() const {return literals.size();}
          [[nodiscard]] inline auto begin() const {return literals.begin();}
          [[nodiscard]] inline auto end() const {return literals.end();}
        private:
          std::vector<expression> literals;
      };
      void addClause(const clause &c);

      clause operator[](int x) const;
      [[nodiscard]] inline auto size() const {return clauses.size();}
      [[nodiscard]] inline auto begin() const {return clauses.begin();}
      [[nodiscard]] inline auto end() const {return clauses.end();}
    private:
      std::vector<clause> clauses;
      std::map<base_operands,std::set<unsigned>> variable_to_clause;

      bool isCNF(const complyer_sat::expression& e);
      void tseytinTransformation(const complyer_sat::expression& e);
      friend class model;
  };

  class model {
    public:
      enum states {FALSE, TRUE, UNKNOWN};
      enum satisfiability {UNSAT, SAT};
      states getAssignment(int x);
      inline satisfiability isSat(){return sat;}
    private:
      model(const cnf &c);
      std::vector<std::pair<complyer_sat::base_operands, states>> setAssignment(base_operands s, states st);
      std::map<std::string, states> assignments;
      std::set<std::string> unique_vars;
      satisfiability sat;
      friend class solver;
  };

  class solver {
    public:
      static model solve(cnf);
    private:
      struct sub_solvers {
        cnf expression;
        virtual bool check(const cnf &c){
          expression = c;
          return true;
        };
        virtual model solve(const cnf &c) = 0;
      };
      struct two_sat : sub_solvers {
        bool check(const cnf &c) override;
        model solve(const cnf &c) override;
      };
      struct horn_sat : sub_solvers {
        bool check(const cnf &c) override;
        model solve(const cnf &c) override;
      };
      struct dpll : sub_solvers {
        model solve(const cnf &c) override;
      };
      struct cdcl : sub_solvers {
        // TODO(anishbadhri): Implement CDCL algorithm
      };
  };
}