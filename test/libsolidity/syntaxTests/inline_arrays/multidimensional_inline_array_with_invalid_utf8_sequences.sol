contract C {
    function f() public pure {
        string[2][2] memory a1 = [['foo', 'bar'], [hex'74000001', hex'c0a80101']];
    }
}
// ----
// TypeError 6069: (110-123): Type literal_string hex"c0a80101" is not implicitly convertible to expected type string memory. Contains invalid UTF-8 sequence at position 4.
