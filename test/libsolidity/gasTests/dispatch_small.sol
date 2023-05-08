contract Small {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    fallback () external payable {}
}
// ----
// creation:
//   codeDepositCost: 103800
//   executionCost: 151
//   totalCost: 103951
// external:
//   fallback: 128
//   a(): 2402
//   b(uint256): infinite
//   f1(uint256): infinite
