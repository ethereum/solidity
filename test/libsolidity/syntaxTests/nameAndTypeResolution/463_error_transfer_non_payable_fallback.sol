// This used to be a test for a.transfer to generate a warning
// because A's fallback function is not payable.

contract A {
    function() public {}
}

contract B {
    A a;

    function() public {
        a.transfer(100);
    }
}
// ----
// Warning: (209-219): Using contract member "transfer" inherited from the address type is deprecated. Convert the contract to "address" type to access the member, for example use "address(contract).transfer" instead.
// TypeError: (209-219): Value transfer to a contract without a payable fallback function.
