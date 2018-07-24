contract Foo {
    uint[] m_x;
    function f() public view {
        uint[] storage memory x = m_x;
        uint[] memory storage calldata y;
        x; y;
    }
}
// ----
// ParserError: (85-91): Location already specified.
// ParserError: (123-130): Location already specified.
// ParserError: (131-139): Location already specified.
