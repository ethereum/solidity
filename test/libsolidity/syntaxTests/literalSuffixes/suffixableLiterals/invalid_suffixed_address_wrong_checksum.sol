function suffix(address value) pure suffix returns (address) { return value; }

contract C {
    address x = 0xffffffffffffffffffffffffffffffffffffffff suffix;
}
// ----
// SyntaxError 9429: (109-158): This looks like an address but has an invalid checksum. Correct checksummed address: "0xFFfFfFffFFfffFFfFFfFFFFFffFFFffffFfFFFfF". If this is not used as an address, please prepend '00'. For more information please see https://docs.soliditylang.org/en/develop/types.html#address-literals
