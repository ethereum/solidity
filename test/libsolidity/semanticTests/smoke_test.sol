contract C {
    function f() public returns (uint) {
        return 2;
    }
    function g(uint x, uint y) public returns (uint) {
        return x - y;
    }
    function h() public payable returns (uint) {
        return f();
    }
    function i(bytes32 b) public returns (bytes32) {
        return b;
    }
    function j(bool b) public returns (bool) {
        return !b;
    }
    function k(bytes32 b) public returns (bytes32) {
        return b;
    }
}
// ----
// f() -> 2
// g(uint256,uint256): 1, -2 -> 3
// h(), 1 ether -> 2
// i() -> FAILURE
// j(bool): true -> false
// k(bytes32): 0x31 -> 0x31
