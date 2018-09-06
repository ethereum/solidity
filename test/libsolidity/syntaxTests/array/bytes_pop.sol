contract C {
    bytes data;
    function test() public {
      data.pop();
    }
}
// ----
