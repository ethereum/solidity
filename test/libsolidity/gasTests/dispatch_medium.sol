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
//   codeDepositCost: 253200
//   executionCost: 294
//   totalCost: 253494
// external:
//   a(): 1028
//   b(uint256): 2046
//   f1(uint256): 41263
//   f2(uint256): 21293
//   f3(uint256): 21337
//   g0(uint256): 313
//   g7(uint256): 21292
//   g8(uint256): 21270
//   g9(uint256): 21226
