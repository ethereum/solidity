function suffix(bytes memory value) pure suffix returns (bytes memory) { return value; }

contract C {
    function f() public pure {
        "" suffix;
        '' suffix;
        "abcd" suffix;
        'abcd' suffix;
        hex"abcd" suffix;
        unicode"😃" suffix;
    }
}
