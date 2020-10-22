contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
}
// ----
// creation:
//   codeDepositCost: 103400
//   executionCost: 153
//   totalCost: 103553
// external:
//   fallback: 129
//   a(): 983
//   b(uint256): 2002
//   f1(uint256): 41263
