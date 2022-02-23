contract C {
  function f() public pure {
    assembly {
      let x := hex"0011"
    }
  }
}
// ----
