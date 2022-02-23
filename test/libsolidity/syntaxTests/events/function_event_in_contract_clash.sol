contract A {
    event dup();
    function dup() public returns (uint) {
        return 1;
    }
}
// ----
// DeclarationError 2333: (34-96): Identifier already declared.
