function suffix(address payable a) pure returns (address payable) { return a; }

contract C {
    function f() public pure {
        0x1234567890123456789012345678901234567890 suffix;
    }
}
// ----
// TypeError 8838: (133-182): The type of the literal cannot be converted to the parameter of the suffix function.
