contract C {
  function f(uint finney) public pure returns (uint szabo) {
    // These used to be denominations.
    szabo = finney;
  }
}