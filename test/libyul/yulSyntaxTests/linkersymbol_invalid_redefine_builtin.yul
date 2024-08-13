{
    let addr := linkersymbol("contract/library.sol:L")
    function linkersymbol(x) {}
}
// ----
// ParserError 5568: (70-82): Cannot use builtin function name "linkersymbol" as identifier name.
