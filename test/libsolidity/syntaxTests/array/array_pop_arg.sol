contract C {
    uint[] data;
    function test() public {
      data.pop(5);
    }
}
// ----
// TypeError 6160: (65-76='data.pop(5)'): Wrong argument count for function call: 1 arguments given but expected 0.
