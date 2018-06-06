contract C {
    function f() view public {
        this.transfer;
    }
}
// ----
// Warning: (52-65): Using contract member "transfer" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).transfer" instead.
// TypeError: (52-65): Value transfer to a contract without a payable fallback function.
