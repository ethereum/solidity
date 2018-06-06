contract C {
    function f() pure public {
        C c;
        c.call;
    }
}
// ----
// Warning: (65-71): Using contract member "call" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).call" instead.
