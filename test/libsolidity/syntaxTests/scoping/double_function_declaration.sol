contract test {
    function fun() public { }
    function fun() public { }
}
// ----
// DeclarationError: Function with same name and arguments defined twice.
