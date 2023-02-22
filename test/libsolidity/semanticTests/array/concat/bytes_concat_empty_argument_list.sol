contract C {
    function f() public returns (bytes memory) {
        return bytes.concat();
    }
}
// ----
// f() -> 0x20, 0
