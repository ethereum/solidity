contract Foo {
    uint[] m_x;
    function f() public view {
        uint[] storage memory x = m_x;
        uint[] memory storage calldata y;
        uint[] storage calldata x2;
        uint[] storage storage x3;
        uint[] calldata memory x4;
        uint[] calldata calldata x5;
        uint[] calldata storage x6;
        uint[] storage memory x4;
        uint[] storage calldata x5;
        uint[] storage storage x6;
        x; y;
    }
}
// ----
// ParserError 3548: (85-91): Location already specified.
// ParserError 3548: (123-130): Location already specified.
// ParserError 3548: (131-139): Location already specified.
// ParserError 3548: (166-174): Location already specified.
// ParserError 3548: (202-209): Location already specified.
// ParserError 3548: (238-244): Location already specified.
// ParserError 3548: (273-281): Location already specified.
// ParserError 3548: (310-317): Location already specified.
// ParserError 3548: (345-351): Location already specified.
// ParserError 3548: (379-387): Location already specified.
// ParserError 3548: (415-422): Location already specified.
