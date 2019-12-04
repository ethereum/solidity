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
//   a(): 983
//   b(uint256): 2002
//   f1(uint256): 41263
