contract test {
    function f() public pure returns (string memory) {
        return "\xc1";
    }
}
// ----
// TypeError 6359: (86-92): Return argument type literal_string hex"c1" is not implicitly convertible to expected type (type of first return variable) string memory. Contains invalid UTF-8 sequence at position 0.
