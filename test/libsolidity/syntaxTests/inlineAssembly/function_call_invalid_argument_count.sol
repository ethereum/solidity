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
// TypeError 7000: (87-88='f'): Function "f" expects 1 arguments but got 0.
// TypeError 7000: (108-109='f'): Function "f" expects 1 arguments but got 2.
