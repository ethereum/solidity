pragma experimental "v0.5.0";
contract test {
    function f() public {
        {
            uint256 x;
        }
        x = 2;
    }
}
// ----
// DeclarationError: Undeclared identifier.
