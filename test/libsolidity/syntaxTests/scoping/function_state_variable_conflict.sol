contract C {
    function f(uint) public pure {}
    uint public f = 0;
}
// ----
// DeclarationError: (53-70): Identifier already declared.
