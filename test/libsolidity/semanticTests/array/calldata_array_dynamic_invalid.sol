pragma abicoder               v2;


contract C {
    function f(uint256[][] calldata a) external returns (uint256) {
        return 42;
    }

    function g(uint256[][] calldata a) external returns (uint256) {
        a[0];
        return 42;
    }
}
// ====
// compileToEwasm: also
// ----
// f(uint256[][]): 0x20, 0x0 -> 42 # valid access stub #
// f(uint256[][]): 0x20, 0x1 -> FAILURE # invalid on argument decoding #
// f(uint256[][]): 0x20, 0x1, 0x20 -> 42 # invalid on outer access #
// g(uint256[][]): 0x20, 0x1, 0x20 -> FAILURE
// f(uint256[][]): 0x20, 0x1, 0x20, 0x2, 0x42 -> 42 # invalid on inner access #
// g(uint256[][]): 0x20, 0x1, 0x20, 0x2, 0x42 -> FAILURE
