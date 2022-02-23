contract test {
    uint256 variable;
    function f(uint256 arg) public {
        f(notfound);
    }
}
// ----
// DeclarationError 7576: (85-93): Undeclared identifier.
