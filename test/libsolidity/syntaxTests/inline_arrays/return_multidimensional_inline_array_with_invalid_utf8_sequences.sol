contract C {
    function g() public pure returns (string[2][2] memory) {
        return [['foo', 'bar'], [hex'74000001', hex'c0a80101']];
    }
}
// ----
// TypeError 6069: (122-135): Type literal_string hex"c0a80101" is not implicitly convertible to expected type string memory. Contains invalid UTF-8 sequence at position 4.
