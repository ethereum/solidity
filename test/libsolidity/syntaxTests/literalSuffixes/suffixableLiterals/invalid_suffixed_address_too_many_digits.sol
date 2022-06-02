function suffix(address value) pure returns (address) { return value; }

contract C {
    address x = 0x12345678901234567890123456789012345678901 suffix;
}
// ----
// SyntaxError 9429: (102-152): This looks like an address but is not exactly 40 hex digits. It is 41 hex digits. If this is not used as an address, please prepend '00'. For more information please see https://docs.soliditylang.org/en/develop/types.html#address-literals
