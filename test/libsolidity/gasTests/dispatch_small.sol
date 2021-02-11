contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
}
// ----
// creation:
//   codeDepositCost: 112800
//   executionCost: 159
//   totalCost: 112959
// external:
//   fallback: 129
//   a(): 1107
//   b(uint256): infinite
//   f1(uint256): infinite
