contract A {
    function dup() public returns (uint) {
        return 1;
    }
}
contract B {
    event dup();
}
contract C is A, B {
}
// ----
// DeclarationError 9097: (99-111): Identifier already declared.
