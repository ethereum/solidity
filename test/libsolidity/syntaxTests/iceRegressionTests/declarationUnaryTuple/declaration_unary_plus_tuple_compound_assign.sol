contract C
{
    function f(int x) public
    {
        (x /= 1) + +(1,1);
    }
}
// ----
// ParserError 9636: (67-68): Use of unary + is disallowed.
