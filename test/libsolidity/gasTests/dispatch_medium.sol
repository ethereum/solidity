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
//   a(): 428
//   b(uint256): 846
//   f1(uint256): 40663
//   f2(uint256): 20693
//   f3(uint256): 20737
//   g0(uint256): 313
//   g7(uint256): 20692
//   g8(uint256): 20670
//   g9(uint256): 20626
