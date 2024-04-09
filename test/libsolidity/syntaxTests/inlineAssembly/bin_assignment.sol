contract C {
  function f() public pure {
    assembly {
      let x := bin"10010110"
    }
  }
}
// ----
