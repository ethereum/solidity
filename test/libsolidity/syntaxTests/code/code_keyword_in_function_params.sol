contract X{
    function foo(string code input) internal pure {
        string code test = input;
    }
}
// ----
// ParserError 2314: (36-40): Expected ',' but got 'code'
