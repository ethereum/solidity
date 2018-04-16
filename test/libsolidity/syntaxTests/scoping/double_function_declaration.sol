contract test {
    function fun() public { }
    function fun() public { }
}
// ----
// DeclarationError: (20-45): Function with same name and arguments defined twice.
