contract test {
    function fun() public {
        uint64(2);
    }
    uint256 foo;
    function foo() public {}
}
// ----
// DeclarationError: (90-114): Identifier already declared.
