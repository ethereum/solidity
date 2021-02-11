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
// ====
// optimize: true
// optimize-runs: 2
// ----
// creation:
//   codeDepositCost: 256200
//   executionCost: 300
//   totalCost: 256500
// external:
//   a(): 983
//   b(uint256): 2337
//   f0(uint256): 366
//   f1(uint256): 41506
//   f2(uint256): 21572
//   f3(uint256): 21660
//   f4(uint256): 21638
//   f5(uint256): 21616
//   f6(uint256): 21528
//   f7(uint256): 21308
//   f8(uint256): 21440
//   f9(uint256): 21462
//   g0(uint256): 606
//   g1(uint256): 41218
//   g2(uint256): 21306
//   g3(uint256): 21394
//   g4(uint256): 21372
//   g5(uint256): 21460
//   g6(uint256): 21240
//   g7(uint256): 21350
//   g8(uint256): 21328
//   g9(uint256): 21174
