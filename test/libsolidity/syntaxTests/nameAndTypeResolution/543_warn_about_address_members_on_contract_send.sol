contract C {
    function f() view public {
        this.send;
    }
}
// ----
// Warning: (52-61): Using contract member "send" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).send" instead.
// TypeError: (52-61): Value transfer to a contract without a payable fallback function.
