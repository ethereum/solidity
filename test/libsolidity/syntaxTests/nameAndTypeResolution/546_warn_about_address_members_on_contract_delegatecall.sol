contract C {
    function f() view public {
        this.delegatecall;
    }
}
// ----
// Warning: (52-69): Using contract member "delegatecall" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).delegatecall" instead.
