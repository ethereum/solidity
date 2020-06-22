contract test {
    function f() public {
        {
            uint256 x;
        }
        x = 2;
    }
}
// ----
// DeclarationError 7576: (93-94): Undeclared identifier.
