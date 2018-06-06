contract C {
    function f() view public {
        C c;
        c.transfer;
    }
}
// ----
// Warning: (65-75): Using contract member "transfer" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).transfer" instead.
// TypeError: (65-75): Value transfer to a contract without a payable fallback function.
