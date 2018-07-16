contract C {
    uint data;
    function test() public {
      data.pop();
    }
}
// ----
// TypeError: (63-71): Member "pop" not found or not visible after argument-dependent lookup in uint256.
