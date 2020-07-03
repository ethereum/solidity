// This tests that the legacy optimiser
// assigns the value of the first function
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
// compileViaYul: false
// ----
// f() -> 0x01
