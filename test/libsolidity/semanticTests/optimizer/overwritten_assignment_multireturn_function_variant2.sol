// This tests that the optimisers
// do not replace a function call
// whose output values have been
// previously assigned to a
// single variable through a
// multi assignment statement with
// a reference to the single variable
contract C {
  function f() public returns (uint o1, uint o2) {
    assembly {
      function g() -> a, b { a := 1 b := 2 }
      let x
      x, x := g()
      o1, o2 := g()
    }
  }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x01, 0x02
