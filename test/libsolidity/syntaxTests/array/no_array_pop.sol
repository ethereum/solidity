contract C {
    uint data;
    function test() public {
      data.pop();
    }
}
// ----
// TypeError 9582: (63-71='data.pop'): Member "pop" not found or not visible after argument-dependent lookup in uint256.
