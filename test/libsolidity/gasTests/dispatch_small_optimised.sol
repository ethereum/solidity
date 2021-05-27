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
//   codeDepositCost: 65800
//   executionCost: 124
//   totalCost: 65924
// external:
//   fallback: 118
//   a(): 2256
//   b(uint256): 4580
//   f1(uint256): 46715
