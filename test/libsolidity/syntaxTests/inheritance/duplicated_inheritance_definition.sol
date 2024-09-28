contract A {}
contract B {}
contract C is A is B{ }
// ----
// ParserError 6668: (44-46): Base contracts were already defined previously.
