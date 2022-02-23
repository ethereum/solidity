function f() pure returns (uint) unchecked {}
// ----
// ParserError 5296: (33-42): "unchecked" blocks can only be used inside regular blocks.
