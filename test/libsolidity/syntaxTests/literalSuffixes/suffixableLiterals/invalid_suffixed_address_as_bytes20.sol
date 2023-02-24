function suffix(bytes20) pure suffix returns (bytes20) {}

contract C {
    function f() public pure {
        0x1234567890123456789012345678901234567890 suffix;
    }
}
// ----
// TypeError 8838: (111-153): The address cannot be converted to type bytes20 accepted by the suffix function.
