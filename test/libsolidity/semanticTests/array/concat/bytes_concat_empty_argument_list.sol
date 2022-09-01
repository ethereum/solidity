contract C {
    function f() public returns (bytes memory) {
        return bytes.concat();
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x20, 0
