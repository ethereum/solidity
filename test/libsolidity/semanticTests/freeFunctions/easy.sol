function add(uint a, uint b) pure returns (uint) {
  return a + b;
}

contract C {
  function f(uint x) public pure returns (uint) {
    return add(x, 2);
  }
}
// ----
// f(uint256): 7 -> 9
