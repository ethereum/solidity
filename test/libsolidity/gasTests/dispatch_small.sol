contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
}
// ----
// creation:
//   codeDepositCost: 84800
//   executionCost: 135
//   totalCost: 84935
// external:
//   fallback: 129
//   a(): 383
//   b(uint256): 802
//   f1(uint256): 40663
