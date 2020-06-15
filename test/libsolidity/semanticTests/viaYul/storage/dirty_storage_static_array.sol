contract C {
    uint8[1] s;
    function f() public returns (bool correct) {
        assembly {
            sstore(s_slot, 257)
        }
        uint8 x = s[0];
        uint r;
        assembly {
            r := x
        }
        correct = (s[0] == 0x01) && (r == 0x01);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true
