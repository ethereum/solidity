contract C {
    function f(bytes calldata b) public returns (bool correct) {
        byte a = b[3];
        uint r;
        assembly {
            r := a
        }
        correct = r == (0x64 << 248);
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0x04, "dead" -> true
