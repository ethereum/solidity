contract A {
    function foo() private {}
    function foo(uint128 value) private {}
    function foo(uint256 value) public {}
}
contract B is A {
    function foo(uint128 value) private {}
    function foo(uint256 value) public {}
}
// ----
TypeError 9456: (195-232): Overriding function is missing "override" specifier.
TypeError 4334: (90-127): Trying to override non-virtual function. Did you forget to add "virtual"?
