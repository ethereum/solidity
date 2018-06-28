// This used to be a test for a.send to generate a warning
// because A does not have a payable fallback function.

contract A {
    function() external {}
}

contract B {
    A a;

    function() external {
        require(a.send(100));
    }
}
// ----
// Warning: (224-230): Using contract member "send" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).send" instead.
// TypeError: (224-230): Value transfer to a contract without a payable fallback function.
