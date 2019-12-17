contract C {
  function f() public pure {
    abi.decode(uint, uint);
  }
}
// ----
// TypeError: (57-61): The first argument to "abi.decode" must be implicitly convertible to bytes memory or bytes calldata, but is of type type(uint256).
// TypeError: (63-67): The second argument to "abi.decode" has to be a tuple of types.
