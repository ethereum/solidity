contract C {
    string data;
    function test() public {
      data.pop();
    }
}
// ----
// TypeError: (65-73): Member "pop" not found or not visible after argument-dependent lookup in string storage ref
