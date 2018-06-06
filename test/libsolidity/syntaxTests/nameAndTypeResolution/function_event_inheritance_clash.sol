contract B {
    event dup();
}
contract A {
    function dup() public returns (uint) {
        return 1;
    }
}
contract C is B, A {
}
// ----
// DeclarationError: (49-111): Identifier already declared.
