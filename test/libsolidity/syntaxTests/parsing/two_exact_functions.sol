// with support of overloaded functions, during parsing,
// we can't determine whether they match exactly, however
// it will throw DeclarationError in following stage.
contract test {
    function fun(uint a) public returns(uint r) { return a; }
    function fun(uint a) public returns(uint r) { return a; }
}
// ----
// DeclarationError: (189-246): Function with same name and arguments defined twice.
