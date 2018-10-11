contract c {
    function f () public
    {
        a = abd;
        a = ade;
    }
    uint256 a;
    uint256 abc;
}
// ----
// DeclarationError: (56-59): Undeclared identifier. Did you mean "abc" or "abi"?
// DeclarationError: (73-76): Undeclared identifier.
