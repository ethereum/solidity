{
    let addr:u256 := linkersymbol("contract/library.sol:L")
    function linkersymbol(x) {}
}
// ====
// dialect: evmTyped
// ----
// ParserError 5568: (75-87): Cannot use builtin function name "linkersymbol" as identifier name.
