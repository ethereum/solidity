pragma abicoder               v2;

contract C {
    function f() public pure returns(string[5] calldata) {
        return ["h", "e", "l", "l", "o"];
    }
}
// ----
// TypeError 6069: (123-126): Type literal_string "h" is not implicitly convertible to expected type string calldata.
// TypeError 6069: (128-131): Type literal_string "e" is not implicitly convertible to expected type string calldata.
// TypeError 6069: (133-136): Type literal_string "l" is not implicitly convertible to expected type string calldata.
// TypeError 6069: (138-141): Type literal_string "l" is not implicitly convertible to expected type string calldata.
// TypeError 6069: (143-146): Type literal_string "o" is not implicitly convertible to expected type string calldata.
// TypeError 6359: (122-147): Return argument type string memory[5] memory is not implicitly convertible to expected type (type of first return variable) string calldata[5] calldata.
