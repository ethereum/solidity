contract C {
    function f() public pure {
        string memory s = [hex'74000001', hex'c0a80101'][1];
    }
}
// ----
// TypeError 6069: (86-99): Type literal_string hex"c0a80101" is not implicitly convertible to expected type string memory. Contains invalid UTF-8 sequence at position 4.
