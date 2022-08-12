contract C {
    function f() public returns (uint256) {
        return ([1, 2, 3, 4][2]);
    }

    function g() public returns (uint256) {
        return [7][0];
    }

    function h(uint i) public returns (int256) {
        return [1, -1][i];
    }

    function k(uint i) public returns (int256) {
        return [-2, 2][i];
    }

    function l(uint i) public returns (int256) {
        int8 a = -2;
        uint8 b = 3;
        return [1, a, b, -1, get()][i];
    }

    function get() internal returns (int) {
        return -2;
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 3
// g() -> 7
// h(uint256): 0 -> 1
// h(uint256): 1 -> -1
// k(uint256): 0 -> -2
// k(uint256): 1 -> 2
// l(uint256): 0 -> 1
// l(uint256): 1 -> -2
// l(uint256): 2 -> 3
// l(uint256): 3 -> -1
// l(uint256): 4 -> -2
