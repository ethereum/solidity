contract B {
    modifier mod1(uint indexed a) { _; }
}
// ----
// ParserError 2314: (36-43): Expected ',' but got 'indexed'
