contract C {
    function f() view public {
        this.call;
    }
}
// ----
// Warning: (52-61): Using contract member "call" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).call" instead.
