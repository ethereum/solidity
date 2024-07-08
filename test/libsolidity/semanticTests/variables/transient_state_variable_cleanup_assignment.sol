contract C {
    uint8 transient x;
    function f() public returns(uint256 r) {
        uint8 y;
        assembly { y := 0xFFFF }
        x = y;
        assembly {
            r := tload(x.slot)
        }

    }
}
// ====
// EVMVersion: >=cancun
// compileViaYul: false
// ----
// f() -> 0xff
