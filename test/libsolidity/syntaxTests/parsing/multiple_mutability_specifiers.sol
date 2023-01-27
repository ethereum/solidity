contract C {
    uint constant constant a;
    uint immutable immutable b;
    uint constant immutable c;
    uint immutable constant d;
}
// ----
// ParserError 3109: (31-39): Mutability already set to "constant"
// ParserError 3109: (62-71): Mutability already set to "immutable"
// ParserError 3109: (93-102): Mutability already set to "constant"
// ParserError 3109: (125-133): Mutability already set to "immutable"
