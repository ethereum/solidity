error E();
function f() public pure {
    revert E;
}
// ----
// ParserError 2314: (50-51): Expected '(' but got ';'
