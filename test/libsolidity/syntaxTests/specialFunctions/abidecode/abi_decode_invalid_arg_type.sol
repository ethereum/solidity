contract C {
  function f() public pure {
    abi.decode(uint, uint);
  }
}
// ----
// TypeError: (57-61): Invalid type for argument in function call. Invalid implicit conversion from type(uint256) to bytes memory requested.
// TypeError: (63-67): The second argument to "abi.decode" has to be a tuple of types.
