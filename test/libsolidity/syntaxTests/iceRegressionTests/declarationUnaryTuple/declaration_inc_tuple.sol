contract C
{
    function f() public
    {
        (int x) = ++(,);
    }
}
// ----
// TypeError: (61-66): Unary operator ++ cannot be applied to type tuple(,)
