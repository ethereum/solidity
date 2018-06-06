contract C {
    function f() pure public {
        address x = 0xA0bFc97E48458494Ccd857e1A85DC91F7F0046E;
        x;
    }
}
// ----
// Warning: (64-105): This looks like an address but has an invalid checksum. If this is not used as an address, please prepend '00'. Correct checksummed address: '0x0A0BfC97E48458494ccD857e1A85Dc91f7f0046e'. For more information please see https://solidity.readthedocs.io/en/develop/types.html#address-literals
