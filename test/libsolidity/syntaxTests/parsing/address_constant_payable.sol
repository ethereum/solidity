contract C {
    address constant payable b = address(0);
}
// ----
// ParserError 2314: (34-41): Expected identifier but got 'payable'
