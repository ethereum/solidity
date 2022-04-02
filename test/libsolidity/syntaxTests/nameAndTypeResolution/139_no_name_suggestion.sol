contract c {
    function g() public { fun(); }
}
// ----
// DeclarationError 7576: (39-42='fun'): Undeclared identifier.
