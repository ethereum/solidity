// This used to be a test for a.transfer to generate a warning
// because A does not have a payable fallback function.

contract A {
    function() payable external {}
}

contract B {
    A a;

    function() external {
        address(a).transfer(100);
    }
}
// ----
