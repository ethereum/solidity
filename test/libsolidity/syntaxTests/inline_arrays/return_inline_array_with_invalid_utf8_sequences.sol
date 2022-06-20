contract C {
    function f() public pure returns (string[2] memory) {
        return [string(bytes(hex'74000001')), string(bytes(hex'c0a80101'))];
    }

    function g() public pure returns (string[2] memory) {
        return [hex'74000001', hex'c0a80101'];
    }
}
// ----
// TypeError 6069: (244-257): Type literal_string hex"c0a80101" is not implicitly convertible to expected type string memory. Contains invalid UTF-8 sequence at position 4.
