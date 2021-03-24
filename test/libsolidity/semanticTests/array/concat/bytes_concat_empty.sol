contract C {
    function f() public returns (bytes memory) {
        return bytes.concat();
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 0
