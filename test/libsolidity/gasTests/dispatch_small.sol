contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    function () external payable {}
}
// ----
// creation:
//   codeDepositCost: 83800
//   executionCost: 135
//   totalCost: 83935
// external:
//   fallback: 118
//   a(): 383
//   b(uint256): 802
//   f1(uint256): 40663
