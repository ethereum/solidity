contract C {
    uint public state = 0;
    constructor(uint _state) public payable {
        state = _state;
    }
    function balance() payable public returns (uint256) {
        return address(this).balance;
    }
    function f() payable public returns (uint) {
        return 2;
    }
    function g() public returns (uint, uint) {
        return (2, 3);
    }
    function h(uint x, uint y) public returns (uint) {
        return x - y;
    }
    function j(bool b) public returns (bool) {
        return !b;
    }
    function k(bytes32 b) public returns (bytes32, bytes32) {
        return (b, b);
    }
    function l() public returns (uint256) {
        return msg.data.length;
    }
    function m(bytes memory b) public returns (bytes memory) {
        return b;
    }
    function n() public returns (string memory) {
        return "any";
    }
    function o() public returns (string memory, string memory) {
        return ("any", "any");
    }
    function p() public returns (string memory, uint, string memory) {
        return ("any", 42, "any");
    }
    function q(uint a) public returns (uint d) {
        return a * 7;
    }
}
// ----
// constructor(), 2 ether: 3 ->
// state() -> 3
// balance() -> 2
// _() -> FAILURE
// f() -> 2
// f(), 1 ether -> 2
// g() -> 2, 3
// h(uint256,uint256): 1, -2 -> 3
// j(bool): true -> false
// k(bytes32): 0x10 -> 0x10, 0x10
// l(): hex"4200ef" -> 7
// m(bytes): 32, 32, 0x20 -> 32, 32, 0x20
// m(bytes): 32, 3, hex"AB33BB" -> 32, 3, left(0xAB33BB)
// m(bytes): 32, 3, hex"AB33FF" -> 32, 3, hex"ab33ff0000000000000000000000000000000000000000000000000000000000"
// n() -> 0x20, 3, "any"
// o() -> 0x40, 0x80, 3, "any", 3, "any"
// p() -> 0x60, 0x2a, 0xa0, 3, "any", 3, "any"
// q(uint256): 0 -> 0
// q(uint256): 99 -> 693
