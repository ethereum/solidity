contract C {
    function f() view public {
        C c;
        c.send;
    }
}
// ----
// Warning: (65-71): Using contract member "send" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).send" instead.
// TypeError: (65-71): Value transfer to a contract without a payable fallback function.
