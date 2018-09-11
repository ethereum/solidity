contract C {
    mapping(address payable => uint) m;
}
// ----
// ParserError: (33-40): Expected '=>' but got 'payable'
