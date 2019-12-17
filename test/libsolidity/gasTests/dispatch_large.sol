contract Large {
    uint public a;
    uint[] public b;
    function f1(uint x) public returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    function f2(uint x) public returns (uint) { b[uint8(msg.data[1])] = x; }
    function f3(uint x) public returns (uint) { b[uint8(msg.data[2])] = x; }
    function f4(uint x) public returns (uint) { b[uint8(msg.data[3])] = x; }
    function f5(uint x) public returns (uint) { b[uint8(msg.data[4])] = x; }
    function f6(uint x) public returns (uint) { b[uint8(msg.data[5])] = x; }
    function f7(uint x) public returns (uint) { b[uint8(msg.data[6])] = x; }
    function f8(uint x) public returns (uint) { b[uint8(msg.data[7])] = x; }
    function f9(uint x) public returns (uint) { b[uint8(msg.data[8])] = x; }
    function f0(uint x) public pure returns (uint) { require(x > 10); }
    function g1(uint x) public payable returns (uint) { a = x; b[uint8(msg.data[0])] = x; }
    function g2(uint x) public payable returns (uint) { b[uint8(msg.data[1])] = x; }
    function g3(uint x) public payable returns (uint) { b[uint8(msg.data[2])] = x; }
    function g4(uint x) public payable returns (uint) { b[uint8(msg.data[3])] = x; }
    function g5(uint x) public payable returns (uint) { b[uint8(msg.data[4])] = x; }
    function g6(uint x) public payable returns (uint) { b[uint8(msg.data[5])] = x; }
    function g7(uint x) public payable returns (uint) { b[uint8(msg.data[6])] = x; }
    function g8(uint x) public payable returns (uint) { b[uint8(msg.data[7])] = x; }
    function g9(uint x) public payable returns (uint) { b[uint8(msg.data[8])] = x; }
    function g0(uint x) public payable returns (uint) { require(x > 10); }
}
// ----
// creation:
//   codeDepositCost: 637000
//   executionCost: 670
//   totalCost: 637670
// external:
//   a(): 1051
//   b(uint256): 2046
//   f0(uint256): 427
//   f1(uint256): 41352
//   f2(uint256): 21293
//   f3(uint256): 21381
//   f4(uint256): 21359
//   f5(uint256): 21337
//   f6(uint256): 21360
//   f7(uint256): 21272
//   f8(uint256): 21272
//   f9(uint256): 21294
//   g0(uint256): 313
//   g1(uint256): 41307
//   g2(uint256): 21270
//   g3(uint256): 21358
//   g4(uint256): 21336
//   g5(uint256): 21292
//   g6(uint256): 21315
//   g7(uint256): 21314
//   g8(uint256): 21292
//   g9(uint256): 21249
