contract C {
    function g() public pure returns (string memory) {
        return string.concat;
    }
}
// ----
// TypeError 6359: (83-96): Return argument type function () pure returns (string memory) is not implicitly convertible to expected type (type of first return variable) string memory.
