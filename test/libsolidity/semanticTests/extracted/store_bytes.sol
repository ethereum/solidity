// this test just checks that the copy loop does not mess up the stack
contract C {
    function save() public returns (uint256 r) {
        r = 23;
        savedData = msg.data;
        r = 24;
    }

    bytes savedData;
}

// ----
// save() -> 24 # empty copy loop #
// save(): "abcdefg" -> 24
