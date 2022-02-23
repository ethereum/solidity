contract C {
    function f(string calldata s) public returns (bytes calldata m) {
        return bytes(s);
    }
}
// ====
// compileViaYul: also
// ----
// f(string): 0x20, 3, "abc" -> 0x20, 3, "abc"
