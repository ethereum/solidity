function suffix(string memory value) pure returns (string memory) { return value; }

contract C {
    function f() public pure {
        "" suffix;
        '' suffix;
        "abcd" suffix;
        'abcd' suffix;
        //hex"abcd" suffix; // Hex literals are only implicitly convertible to bytes
        unicode"😃" suffix;
    }
}
