// This used to be a test for a.transfer to generate a warning
// because A's fallback function is not payable.

contract A {
    function() external {}
}

contract B {
    A a;

    function() external {
        address(a).transfer(100);
    }
}
// ----
