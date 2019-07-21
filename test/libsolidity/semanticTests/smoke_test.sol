pragma experimental ABIEncoderV2;

contract C {
    struct S {
        uint a;
        uint b;
    }
    struct T {
        uint a;
        uint b;
        string s;
    }
    uint public state = 0;
    bool[2][] flags;
    constructor(uint _state) public payable {
        state = _state;
    }
    function balance() payable public returns (uint256) {
        return address(this).balance;
    }
    function e(uint a) public {
        state = a;
    }
    function f() payable public returns (uint) {
        return 2;
    }
    function f(uint a) public returns (uint, uint) {
        return (a, a);
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
    function r() public returns (bool[3] memory) {
        return [true, false, true];
    }
    function s() public returns (uint[2] memory, uint) {
        return ([uint(123), 456], 789);
    }
    function t1() public returns (S memory) {
        return S(23, 42);
    }
    function t2() public returns (T memory) {
        return T(23, 42, "any");
    }
    function u() public returns (T[2] memory) {
        return [T(23, 42, "any"), T(555, 666, "any")];
    }
    function v() public returns (bool[2][] memory) {
        return flags;
    }
    function w1() public returns (string[1] memory) {
        return ["any"];
    }
    function w2() public returns (string[2] memory) {
        return ["any", "any"];
    }
    function w3() public returns (string[3] memory) {
        return ["any", "any", "any"];
    }
    function x() public returns (string[2] memory, string[3] memory) {
        return (["any", "any"], ["any", "any", "any"]);
    }
}
// ----
// constructor(), 2 ether: 3 ->
// state() -> 3
// balance() -> 2
// _() -> FAILURE
// e(uint256): 4
// f() -> 2
// f(uint256): 3 -> 3, 3
// f(), 1 ether -> 2
// g() -> 2, 3
// g1() -> FAILURE
// h(uint256,uint256): 1, -2 -> 3
// j(bool): true -> false
// k(bytes32): 0x10 -> 0x10, 0x10
// l(): hex"4200efef" -> 8
// m(bytes): 32, 32, 0x20 -> 32, 32, 0x20
// m(bytes): 32, 3, hex"AB33BB" -> 32, 3, left(0xAB33BB)
// m(bytes): 32, 3, hex"AB33FF" -> 32, 3, hex"ab33ff0000000000000000000000000000000000000000000000000000000000"
// n() -> 0x20, 3, "any"
// o() -> 0x40, 0x80, 3, "any", 3, "any"
// p() -> 0x60, 0x2a, 0xa0, 3, "any", 3, "any"
// q(uint256): 99 -> 693
// r() -> true, false, true
// s() -> 123, 456, 789
// t1() -> 23, 42
// t2() -> 0x20, 23, 42, 0x60, 3, "any"
// v() -> 32, 0
// w1() -> 0x20, 0x20, 3, "any"
// w2() -> 0x20, 0x40, 0x80, 3, "any", 3, "any"
// w3() -> 0x20, 0x60, 0xa0, 0xe0, 3, "any", 3, "any", 3, "any"
// x() -> 0x40, 0x0100, 0x40, 0x80, 3, "any", 3, "any", 0x60, 0xa0, 0xe0, 3, "any", 3, "any", 3, "any"
