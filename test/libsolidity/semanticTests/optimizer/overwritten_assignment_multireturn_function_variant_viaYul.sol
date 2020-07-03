// This tests that the yul optimiser
// assigns the value of the last function
// output to the variable on the LHS of
// an inline assembly multi variable assignment
// that contains the same variable more
// than once
contract C {
  function f() public returns (uint o) {
    assembly {
      function g() -> a, b { a := 1 b := 2 }
      let x
      x, x := g()
      o := x
    }
  }
}
// ====
// compileViaYul: true
// ----
// f() -> 0x02
