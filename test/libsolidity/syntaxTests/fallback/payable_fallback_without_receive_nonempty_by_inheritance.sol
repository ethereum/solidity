contract A {
    function f() external pure {}
}
contract C is A {
    fallback() external payable { }
}
// ----
// Warning: (49-104): This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.
