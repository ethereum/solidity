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
// TypeError: (87-88): Function expects 1 arguments but got 0.
// TypeError: (108-109): Function expects 1 arguments but got 2.
