contract C {
  function g() public pure {
    assembly {
      function f(a) {}

      f()
      f(1)
      f(1, 2)
    }
  }
}
// ----
// TypeError 7000: (87-88): Function "f" expects 1 arguments but got 0.
// TypeError 7000: (108-109): Function "f" expects 1 arguments but got 2.
