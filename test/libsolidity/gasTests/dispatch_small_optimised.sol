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
//   codeDepositCost: 58200
//   executionCost: 109
//   totalCost: 58309
// external:
//   fallback: 117
//   a(): 2259
//   b(uint256): 4582
//   f1(uint256): 46716
