// This used to be a test for a.send to generate a warning
// because A does not have a payable fallback function.

contract A {
    function() public {}
}

contract B {
    A a;

    function() public {
        require(a.send(100));
    }
}
// ----
// Warning: (220-226): Using contract member "send" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).send" instead.
// TypeError: (220-226): Value transfer to a contract without a payable fallback function.
