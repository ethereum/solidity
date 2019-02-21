contract C {
    function f() public returns (uint) {
        return 1;
    }
    function g(uint x, uint y) public returns (uint) {
        return x - y;
    }
    function h() public payable returns (uint) {
        return f();
    }
    function x(bytes32 b) public returns (bytes32) {
        return b;
    }
}
// ----
// f() -> 1
// g(uint256,uint256): 1, -2 -> 3
// h(), 1 ether -> 1
// j() -> FAILURE
// i() # Does not exist. # -> FAILURE # Reverts. #
// x(bytes32): 0x31 -> 0x31
