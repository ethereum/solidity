contract C {
    uint[3] data;
    function test() public {
      data.pop();
    }
}
// ----
// TypeError: (66-74): Member "pop" not found or not visible after argument-dependent lookup in uint256[3] storage ref.
