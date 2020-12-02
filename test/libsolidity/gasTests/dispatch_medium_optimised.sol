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
//   codeDepositCost: 161000
//   executionCost: 208
//   totalCost: 161208
// external:
//   a(): 1028
//   b(uint256): 2128
//   f1(uint256): 41319
//   f2(uint256): 21363
//   f3(uint256): 21407
//   g0(uint256): 397
//   g7(uint256): 21273
//   g8(uint256): 21251
//   g9(uint256): 21207
