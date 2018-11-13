contract C {
  function f() public pure {
    assembly {
      function k() {}

      k
    }
  }
}
// ----
// TypeError: (86-87): Function k used without being called.
