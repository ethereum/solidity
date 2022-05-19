contract A {
    fallback (bytes calldata _input) external returns (bytes memory) {
        return _input;
    }
    function f() public returns (bool, bytes memory) {
        (bool success, bytes memory retval) = address(this).call("abc");
        return (success, retval);
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f() -> 0x01, 0x40, 0x03, 0x6162630000000000000000000000000000000000000000000000000000000000
