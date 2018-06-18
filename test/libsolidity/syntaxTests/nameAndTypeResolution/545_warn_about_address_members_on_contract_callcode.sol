contract C {
    function f() view public {
        this.callcode;
    }
}
// ----
// Warning: (52-65): Using contract member "callcode" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).callcode" instead.
// TypeError: (52-65): "callcode" has been deprecated in favour of "delegatecall".
