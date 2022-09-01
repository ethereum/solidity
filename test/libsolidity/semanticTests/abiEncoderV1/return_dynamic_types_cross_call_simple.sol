contract C {
    function dyn() public returns (bytes memory) {
        return "1234567890123456789012345678901234567890";
    }
    function f() public returns (bytes memory) {
        return this.dyn();
    }
}
// ====
// EVMVersion: >homestead
// ----
// f() -> 0x20, 40, "12345678901234567890123456789012", "34567890"
