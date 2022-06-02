function suffix(address value) pure returns (address) { return value; }

contract C {
    address x = 0x123456789012345678901234567890123456789 suffix;
}
// ----
// SyntaxError 9429: (102-150): This looks like an address but is not exactly 40 hex digits. It is 39 hex digits. If this is not used as an address, please prepend '00'. For more information please see https://docs.soliditylang.org/en/develop/types.html#address-literals
