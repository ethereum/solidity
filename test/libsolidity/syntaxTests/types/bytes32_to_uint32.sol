contract C {
    function f() public pure returns(uint32) {
        return uint32(bytes32(''));
    }
}
// ----
// TypeError: (75-94): Explicit type conversion not allowed from "bytes32" to "uint32".
