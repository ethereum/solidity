contract C {
    function f(bytes calldata c) public returns (string calldata s) {
        return string(c);
    }
}
// ----
// f(bytes): 0x20, 3, "abc" -> 0x20, 3, "abc"
