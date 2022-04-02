contract C {
    function f(uint) public pure {}
    uint public f = 0;
}
// ----
// DeclarationError 2333: (53-70='uint public f = 0'): Identifier already declared.
