// This used to be a test for a.transfer to generate a warning
// because A does not have a payable fallback function.

contract A {
    function() payable public {}
}

contract B {
    A a;

    function() public {
        a.transfer(100);
    }
}
// ----
// Warning: (224-234): Using contract member "transfer" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).transfer" instead.
