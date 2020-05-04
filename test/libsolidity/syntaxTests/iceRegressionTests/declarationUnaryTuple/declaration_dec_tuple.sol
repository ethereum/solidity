contract C
{
    function f() public
    {
        int x = --(,);
    }
}
// ----
// TypeError: (59-64): Unary operator -- cannot be applied to type tuple(,)
