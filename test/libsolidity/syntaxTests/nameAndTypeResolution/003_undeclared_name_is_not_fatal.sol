contract test {
    uint256 variable;
    function f(uint256 arg) public {
        f(notfound);
        f(notfound);
    }
}
// ----
// DeclarationError 7576: (85-93='notfound'): Undeclared identifier.
// DeclarationError 7576: (106-114='notfound'): Undeclared identifier.
