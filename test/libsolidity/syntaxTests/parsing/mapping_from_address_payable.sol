contract C {
    mapping(address payable => uint) m;
}
// ----
// ParserError 2314: (33-40='payable'): Expected '=>' but got 'payable'
