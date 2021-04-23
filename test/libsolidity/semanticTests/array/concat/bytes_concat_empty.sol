contract C {
    function f() public returns (bytes memory) {
        return bytes.concat();
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x20, 0
