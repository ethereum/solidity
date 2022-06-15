contract C {
    bytes x;
    function f() public returns (uint r) {
        bytes memory m = "tmp";
        assembly {
            mstore(m, 8)
            mstore(add(m, 32), "deadbeef15dead")
        }
        // via yul disabled because this truncates the string.
        x = m;
        assembly {
            r := sload(x.slot)
        }
    }
}
// ====
// compileViaYul: false
// ----
// f() -> 0x6465616462656566000000000000000000000000000000000000000000000010
