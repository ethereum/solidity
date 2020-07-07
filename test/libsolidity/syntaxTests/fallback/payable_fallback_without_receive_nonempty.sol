contract C {
    fallback() external payable { }
    function f() public pure { }
}
// ----
// Warning 3628: (0-83): This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.
