contract Medium {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    function f2(uint x) public returns (uint) { b[uint8(msg.data[1])] = x; }
    function f3(uint x) public returns (uint) { b[uint8(msg.data[2])] = x; }
    function g7(uint x) public payable returns (uint) { b[uint8(msg.data[6])] = x; }
    function g8(uint x) public payable returns (uint) { b[uint8(msg.data[7])] = x; }
    function g9(uint x) public payable returns (uint) { b[uint8(msg.data[8])] = x; }
    function g0(uint x) public payable returns (uint) { require(x > 10); }
}
// ----
// creation:
//   codeDepositCost: 360400
//   executionCost: 399
//   totalCost: 360799
// external:
//   a(): 1152
//   b(uint256): infinite
//   f1(uint256): infinite
//   f2(uint256): infinite
//   f3(uint256): infinite
//   g0(uint256): infinite
//   g7(uint256): infinite
//   g8(uint256): infinite
//   g9(uint256): infinite
