contract C {
        function f1() external pure returns (string[1] memory rdatas) {
                rdatas = [hex'c0a80101'];
        }
}
// ----
// TypeError 7407: (110-125): Type inline_array(literal_string hex"c0a80101") is not implicitly convertible to expected type string memory[1] memory. Invalid conversion from literal_string hex"c0a80101" to string memory. Contains invalid UTF-8 sequence at position 4.
