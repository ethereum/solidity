contract C {
    function f() public pure returns (bool correct) {
        uint256[1] memory m;
        assembly {
            mstore(m, 0xdeadbeef15dead)
        }
        uint32 x = uint32(m[0]);
        uint r;
        assembly {
            r := x
        }
        correct = (r == 0xef15dead) && (m[0] == 0xdeadbeef15dead);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: true
// ----
// f() -> true
