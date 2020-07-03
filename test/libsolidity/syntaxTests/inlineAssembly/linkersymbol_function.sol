contract C {
  function f() public pure {
    assembly {
      function linkersymbol(a) {}

      linkersymbol("contract/library.sol:L")
    }
  }
}
// ----
