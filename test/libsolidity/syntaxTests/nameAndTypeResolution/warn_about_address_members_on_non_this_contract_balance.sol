contract C {
    function f() view public {
        C c;
        c.balance;
    }
}
// ----
// Warning: (65-74): Using contract member "balance" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).balance" instead.
