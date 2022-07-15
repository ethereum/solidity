contract C {
    function f() external pure {
        string[2] memory a1 = [hex'74000001', hex'c0a80101'];
        string[2] memory a2 = [bytes(hex'74000001'), bytes(hex'c0a80101')];
        bytes[2] memory a3 = [hex'74000001', hex'c0a80101'];
        bytes[2] memory a4 = ['foo', 'bar'];
    }
}
// ----
// TypeError 9574: (54-106): Type inline_array(literal_string hex"74000001", literal_string hex"c0a80101") is not implicitly convertible to expected type string[2] memory. Invalid conversion from literal_string hex"c0a80101" to string memory. Contains invalid UTF-8 sequence at position 4.
// TypeError 9574: (116-182): Type inline_array(bytes memory, bytes memory) is not implicitly convertible to expected type string[2] memory. Invalid conversion from bytes memory to string memory.
