contract C {
    function f() pure public {
        address x = 0xFA0bFc97E48458494Ccd857e1A85DC91F7F0046E0;
        x;
    }
}
// ----
// SyntaxError: (64-107): This looks like an address but is not exactly 40 hex digits. It is 41 hex digits. If this is not used as an address, please prepend '00'. For more information please see https://solidity.readthedocs.io/en/develop/types.html#address-literals
