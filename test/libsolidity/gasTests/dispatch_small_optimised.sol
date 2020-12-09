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
//   codeDepositCost: 76200
//   executionCost: 123
//   totalCost: 76323
// external:
//   fallback: 118
//   a(): 1006
//   b(uint256): 2018
//   f1(uint256): 41253
