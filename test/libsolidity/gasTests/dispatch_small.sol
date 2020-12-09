contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
}
// ----
// creation:
//   codeDepositCost: 123600
//   executionCost: 171
//   totalCost: 123771
// external:
//   fallback: 129
//   a(): 1107
//   b(uint256): infinite
//   f1(uint256): infinite
