contract C
{
    function f() public
    {
        (int x) = ++(,);
    }
}
// ----
// TypeError 9767: (61-66): Built-in unary operator ++ cannot be applied to type tuple(,).
