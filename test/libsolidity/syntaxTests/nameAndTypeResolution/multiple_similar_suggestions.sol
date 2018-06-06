contract c {
    function g() public {
        uint var1 = 1;
        uint var2 = 1;
        uint var3 = 1;
        uint var4 = 1;
        uint var5 = varx;
    }
}
// ----
// DeclarationError: (151-155): Undeclared identifier. Did you mean "var1", "var2", "var3", "var4" or "var5"?
