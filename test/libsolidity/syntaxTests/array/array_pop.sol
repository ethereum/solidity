contract C {
    uint[] data;
    function test() public {
      data.pop();
    }
}
// ----
