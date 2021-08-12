contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
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
//   a(): 2261
//   b(uint256): 4574
//   f1(uint256): 46709
