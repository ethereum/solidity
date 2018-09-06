contract C {
  function f() public pure {
    assembly {
      function f(a) {}

      f()
      f(1)
      f(1, 2)
    }
  }
}
// ----
// TypeError: (87-88): Expected 1 arguments but got 0.
// SyntaxError: (87-90): Top-level expressions are not supposed to return values (this expression returns -1 values). Use ``pop()`` or assign them.
// TypeError: (108-109): Expected 1 arguments but got 2.
// SyntaxError: (108-115): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
