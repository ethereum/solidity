contract C
{
    function f() public
    {
        int x = +(0, 0);
    }
}
// ----
// ParserError 9636: (59-60): Use of unary + is disallowed.
