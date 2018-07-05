// This used to be a test for a.send to generate a warning
// because A does not have a payable fallback function.

contract A {
    function() external {}
}

contract B {
    A a;

    function() external {
        require(address(a).send(100));
    }
}
// ----
