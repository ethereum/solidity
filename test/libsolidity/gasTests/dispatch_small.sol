contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
}
// ----
// creation:
//   codeDepositCost: 114600
//   executionCost: 159
//   totalCost: 114759
// external:
//   fallback: 129
//   a(): 2407
//   b(uint256): infinite
//   f1(uint256): infinite
