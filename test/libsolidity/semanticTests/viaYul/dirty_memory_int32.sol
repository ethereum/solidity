contract C {
    function f() public pure returns (bool correct) {
        uint256[1] memory m;
        assembly {
            mstore(m, 0xdeadbeef15dead)
        }
        int32 x = int32(uint32(m[0]));
        uint r;
        assembly {
            r := x
        }
        correct = (m[0] == 0xdeadbeef15dead) && (r == (((2 ** 224 - 1) << 32) | 0xef15dead));
    }
}
// ====
// compileViaYul: true
// compileToEwasm: also
// ----
// f() -> true
