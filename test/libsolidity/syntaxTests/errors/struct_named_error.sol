struct error {uint a;}
contract C {
    error x;
}
// ----
// ParserError 2314: (7-12): Expected identifier but got 'error'
