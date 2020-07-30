#include <complyer-sat.h>
#include "catch.hpp"

using namespace complyer_sat;

TEST_CASE("Expression Constructors", "[expression]") {
  SECTION("Base Constructor"){
    SECTION("Non-positive number") {
      REQUIRE_THROWS(expression(0));
      REQUIRE_THROWS(expression(-1));
    }
    SECTION("Positive number") {
      auto x = expression(1);
      REQUIRE(x.getType() == expression::BASE);
      REQUIRE(x.getName() == std::to_string(1));
    }
  }
  SECTION("NOT Constructor") {
    auto e = expression(1);
    auto x = expression(expression::NOT, e);
    REQUIRE(x.getType() == expression::NOT);
    REQUIRE(x.getName() == "(!1)");
    REQUIRE(x.child(0).getName() == e.getName());
  }
  SECTION("AND Constructor") {
    auto e1 = expression(1), e2 = expression(2);
    auto x = expression(expression::AND, e1, e2);
    REQUIRE(x.getType() == expression::AND);
    REQUIRE(x.getName() == "(1 & 2)");
    REQUIRE(x.child(0).getName() == e1.getName());
    REQUIRE(x.child(1).getName() == e2.getName());
  }
  SECTION("OR Constructor") {
    auto e1 = expression(1), e2 = expression(2);
    auto x = expression(expression::OR, e1, e2);
    REQUIRE(x.getType() == expression::OR);
    REQUIRE(x.getName() == "(1 & 2)");
    REQUIRE(x.child(0).getName() == e1.getName());
    REQUIRE(x.child(1).getName() == e2.getName());
  }
}

TEST_CASE("Expression Operations","[expression]"){
  auto e1 = expression(1), e2 = expression(2);
  SECTION("AND Operation") {
    SECTION("Operator") {
      auto eo = e1 && e2;
      REQUIRE(eo.getType() == expression::AND);
      REQUIRE(eo.child(0).getName() == e1.getName());
      REQUIRE(eo.child(1).getName() == e2.getName());
    }
    SECTION("Function") {
      auto ef = l_and(e1, e2);
      REQUIRE(ef.getType() == expression::AND);
      REQUIRE(ef.child(0).getName() == e1.getName());
      REQUIRE(ef.child(1).getName() == e2.getName());
    }
  }
  SECTION("OR Operation") {
    SECTION("Operator") {
      auto eo = e1 || e2;
      REQUIRE(eo.getType() == expression::OR);
      REQUIRE(eo.child(0).getName() == e1.getName());
      REQUIRE(eo.child(1).getName() == e2.getName());
    }
    SECTION("Function") {
      auto ef = l_or(e1, e2);
      REQUIRE(ef.getType() == expression::OR);
      REQUIRE(ef.child(0).getName() == e1.getName());
      REQUIRE(ef.child(1).getName() == e2.getName());
    }
  }
  SECTION("NOT Operation") {
    SECTION("Operator") {
      auto eo = !e1;
      REQUIRE(eo.getType() == expression::NOT);
      REQUIRE(eo.child(0).getName() == e1.getName());
    }SECTION("Function") {
      auto ef = l_not(e1);
      REQUIRE(ef.getType() == expression::NOT);
      REQUIRE(ef.child(0).getName() == e1.getName());
    }
  }
  SECTION("IMPLIES Operation") {
    SECTION("Function") {
      auto ef = implies(e1, e2);
      REQUIRE(ef.getType() == expression::OR);
      REQUIRE(ef.child(0).getName() == l_not(e1).getName());
      REQUIRE(ef.child(1).getName() == e2.getName());
    }
  }
}

TEST_CASE("Expression Functions", "[expression]") {
  auto e1 = expression(1), e2 = expression(2);
  SECTION("NNF"){
    SECTION("AND Operation") {
      auto e = !(e1 && e2);
      auto ef = (!e1 || !e2);
      REQUIRE(e.NNF().getName() == ef.getName());
    }
    SECTION("OR Operation") {
      auto e = !(e1 || e2);
      auto ef = (!e1 && !e2);
      REQUIRE(e.NNF().getName() == ef.getName());
    }
    SECTION("NOT Operation") {
      auto e = !!e1;
      REQUIRE(e.NNF().getName() == e1.getName());
    }
  }
}