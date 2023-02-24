function suffix(address payable a) pure suffix returns (address payable) { return a; }

contract C {
    function f() public pure {
        0x1234567890123456789012345678901234567890 suffix;
    }
}
// ----
// TypeError 2998: (16-33): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 8838: (140-182): The address cannot be converted to type address payable accepted by the suffix function.
