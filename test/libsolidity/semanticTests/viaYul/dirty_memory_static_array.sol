contract C {
    function f() public pure returns (bool correct) {
        uint8[1] memory m;
        assembly {
            mstore(m, 257)
        }
        uint8 x = m[0];
        uint r;
        assembly {
            r := x
        }
        correct = (m[0] == 0x01) && (r == 0x01);
    }
}
// ====
// compileViaYul: true
// ----
// f() -> true
