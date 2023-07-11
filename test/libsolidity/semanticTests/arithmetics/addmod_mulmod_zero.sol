contract C {
    function f(uint256 d) public pure returns (uint256) {
        addmod(1, 2, d);
        return 2;
    }

    function g(uint256 d) public pure returns (uint256) {
        mulmod(1, 2, d);
        return 2;
    }

    function h() public pure returns (uint256) {
        mulmod(0, 1, 2);
        mulmod(1, 0, 2);
        addmod(0, 1, 2);
        addmod(1, 0, 2);
        return 2;
    }
}
// ----
// f(uint256): 0 -> FAILURE, hex"4e487b71", 0x12
// g(uint256): 0 -> FAILURE, hex"4e487b71", 0x12
// h() -> 2
