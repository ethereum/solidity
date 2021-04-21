contract A {
    bytes public x;
    fallback (bytes calldata _input) external returns (bytes memory) {
        x = _input;
        return "";
    }
    function f() public returns (bool, bytes memory) {
        (bool success, bytes memory retval) = address(this).call("abc");
        return (success, retval);
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// f() -> 0x01, 0x40, 0x00
// x() -> 0x20, 3, "abc"
