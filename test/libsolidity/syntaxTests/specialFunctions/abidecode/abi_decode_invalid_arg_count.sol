contract C {
  function f() public pure {
    abi.decode();
    abi.decode(msg.data);
    abi.decode(msg.data, uint, uint);
  }
}
// ----
// TypeError 5782: (46-58='abi.decode()'): This function takes two arguments, but 0 were provided.
// TypeError 5782: (64-84='abi.decode(msg.data)'): This function takes two arguments, but 1 were provided.
// TypeError 5782: (90-122='abi.decode(msg.data, uint, uint)'): This function takes two arguments, but 3 were provided.
// TypeError 6444: (111-115='uint'): The second argument to "abi.decode" has to be a tuple of types.
