function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 0x1234567890123456789012345678901234567890 suffix;
}
// ----
// TypeError 8838: (100-142): The address cannot be converted to type uint256 accepted by the suffix function.
