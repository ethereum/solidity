contract C {
    address public payable a;
}
// ----
// ParserError: (32-39): Expected identifier but got 'payable'
// ParserError: (40-41): Expected ';' but got identifier
// ParserError: (41-42): Expected identifier but got ';'
// ParserError: (43-44): Expected ';' but got '}'
