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
// ====
// optimize: true
// optimize-runs: 2
// ----
// creation:
//   codeDepositCost: 126000
//   executionCost: 169
//   totalCost: 126169
// external:
//   a(): 2281
//   b(uint256): 4692
//   f1(uint256): 46782
//   f2(uint256): 24725
//   f3(uint256): 24769
//   g0(uint256): 361
//   g7(uint256): 24635
//   g8(uint256): 24613
//   g9(uint256): 24569
