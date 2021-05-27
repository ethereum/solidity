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
//   codeDepositCost: 137000
//   executionCost: 190
//   totalCost: 137190
// external:
//   a(): 2278
//   b(uint256): 4690
//   f1(uint256): 46781
//   f2(uint256): 24725
//   f3(uint256): 24769
//   g0(uint256): 364
//   g7(uint256): 24640
//   g8(uint256): 24618
//   g9(uint256): 24574
