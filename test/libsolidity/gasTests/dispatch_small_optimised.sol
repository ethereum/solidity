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
//   codeDepositCost: 65400
//   executionCost: 117
//   totalCost: 65517
// external:
//   fallback: 118
//   a(): 376
//   b(uint256): 753
//   f1(uint256): 40600
