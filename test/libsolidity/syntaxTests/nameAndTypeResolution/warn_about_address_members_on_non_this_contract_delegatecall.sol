contract C {
    function f() pure public {
        C c;
        c.delegatecall;
    }
}
// ----
// Warning: (65-79): Using contract member "delegatecall" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).delegatecall" instead.
