contract C {
    uint[] data;
    function test() public {
      data.pop(5);
    }
}
// ----
// TypeError: (65-76): Wrong argument count for function call: 1 arguments given but expected 0.
