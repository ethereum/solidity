contract C {
    function test() public {
      uint[] memory data;
      data.pop();
    }
}
// ----
// TypeError 4994: (74-82='data.pop'): Member "pop" is not available in uint256[] memory outside of storage.
