contract C {
    function f() public returns (bytes memory) {
        return bytes.concat(0, -0, 0.0, -0.0, 0e10, 0e-10, 0x00, (0));
    }

    function g() public returns (bytes memory) {
        return bytes.concat(0, "abc", 0, "abc", 0);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x20, 8, "\0\0\0\0\0\0\0\0"
// g() -> 0x20, 9, "\0abc\0abc\0"
