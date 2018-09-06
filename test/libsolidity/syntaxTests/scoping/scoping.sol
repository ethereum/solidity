contract test {
    function f() public {
        {
            uint256 x;
        }
        x = 2;
    }
}
// ----
// DeclarationError: (93-94): Undeclared identifier.
