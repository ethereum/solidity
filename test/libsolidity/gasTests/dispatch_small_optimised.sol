contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    function () external payable {}
}
// ====
// optimize: true
// optimize-runs: 2
// ----
// creation:
//   codeDepositCost: 60400
//   executionCost: 111
//   totalCost: 60511
// external:
//   fallback: 118
//   a(): 976
//   b(uint256): 1953
//   f1(uint256): 41188
