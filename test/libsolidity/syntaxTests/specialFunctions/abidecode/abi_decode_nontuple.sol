contract C {
  function f() public pure {
    abi.decode("abc", uint);
    abi.decode("abc", this);
    abi.decode("abc", f());
  }
}
// ----
// TypeError: (64-68): The second argument to "abi.decode" has to be a tuple of types.
// TypeError: (93-97): The second argument to "abi.decode" has to be a tuple of types.
// TypeError: (122-125): The second argument to "abi.decode" has to be a tuple of types.
