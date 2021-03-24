contract C {
    function g() public pure returns (bytes memory) {
        return bytes.concat;
    }
}
// ----
// TypeError 6359: (82-94): Return argument type function () pure returns (bytes memory) is not implicitly convertible to expected type (type of first return variable) bytes memory.
