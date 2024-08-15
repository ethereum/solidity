{
    function f(a: invalidType) -> b: invalidType {}
}
// ----
// ParserError 5473: (17-31): Types are not valid in untyped Yul.
// ParserError 5473: (36-50): Types are not valid in untyped Yul.
