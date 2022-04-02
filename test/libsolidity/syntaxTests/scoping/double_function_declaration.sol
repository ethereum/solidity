contract test {
    function fun() public { }
    function fun() public { }
}
// ----
// DeclarationError 1686: (20-45='function fun() public { }'): Function with same name and parameter types defined twice.
