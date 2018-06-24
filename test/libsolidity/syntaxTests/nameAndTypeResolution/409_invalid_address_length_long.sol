contract C {
    function f() pure public {
        address x = 0xFA0bFc97E48458494Ccd857e1A85DC91F7F0046E0;
        x;
    }
}
// ----
// Warning: (64-107): This looks like an address but has an invalid checksum. If this is not used as an address, please prepend '00'. For more information please see https://solidity.readthedocs.io/en/develop/types.html#address-literals
// TypeError: (52-107): Type int_const 2284...(42 digits omitted)...9360 is not implicitly convertible to expected type address.
