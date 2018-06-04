contract A {
    event dup();
    function dup() public returns (uint) {
        return 1;
    }
}
// ----
// DeclarationError: (34-96): Identifier already declared.
