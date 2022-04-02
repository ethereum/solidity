contract C {
  function f() public pure {
    abi.decode("abc", uint);
    abi.decode("abc", this);
    abi.decode("abc", f());
  }
}
// ----
// TypeError 6444: (64-68='uint'): The second argument to "abi.decode" has to be a tuple of types.
// TypeError 6444: (93-97='this'): The second argument to "abi.decode" has to be a tuple of types.
// TypeError 6444: (122-125='f()'): The second argument to "abi.decode" has to be a tuple of types.
