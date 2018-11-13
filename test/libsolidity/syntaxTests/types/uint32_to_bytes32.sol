contract C {
    function f() public pure returns(bytes32) {
        return bytes32(uint32(0));
    }
}
// ----
// TypeError: (76-94): Explicit type conversion not allowed from "uint32" to "bytes32".
