pragma experimental ABIEncoderV2;

contract C {
    function e() public payable returns (uint) {
        return msg.value;
    }
    function f(uint a) public pure returns (uint, uint) {
        return (a, a);
    }
    function g() public  pure returns (uint, uint) {
        return (2, 3);
    }
    function h(uint x, uint y) public  pure returns (uint) {
        return x - y;
    }
    function i(bool b) public  pure returns (bool) {
        return !b;
    }
    function j(bytes32 b) public pure returns (bytes32, bytes32) {
        return (b, b);
    }
    function k() public pure returns (uint) {
        return msg.data.length;
    }
    function l(uint a) public pure returns (uint d) {
        return a * 7;
    }
}
// ----
// e(), 1 ether -> 1
// f(uint256): 3 -> 3, 3
// g() -> 2, 3
// h(uint256,uint256): 1, -2 -> 3
// i(bool): true -> false
// j(bytes32): 0x10001 -> 0x10001, 0x10001
// k(): hex"4200efef" -> 8
// l(uint256): 99 -> 693
