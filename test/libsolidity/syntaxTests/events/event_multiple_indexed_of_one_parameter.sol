contract c {
    event e(uint indexed a, bytes3 indexed indexed s, bool indexed indexed indexed b);
    event e2(uint indexed indexed a, bytes3 indexed s);
}
// ----
// ParserError 5399: (56-63): Indexed already specified.
// ParserError 5399: (80-87): Indexed already specified.
// ParserError 5399: (88-95): Indexed already specified.
// ParserError 5399: (126-133): Indexed already specified.
