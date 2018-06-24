contract C {
    function f() pure public {
        C c;
        c.callcode;
    }
}
// ----
// Warning: (65-75): Using contract member "callcode" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).callcode" instead.
// TypeError: (65-75): "callcode" has been deprecated in favour of "delegatecall".
