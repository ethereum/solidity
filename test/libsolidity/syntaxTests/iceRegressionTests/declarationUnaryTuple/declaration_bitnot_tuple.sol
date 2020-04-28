contract C
{
    function f() public
    {
        int x = ~(0,);
    }
}
// ----
// TypeError: (60-64): Tuple component cannot be empty.
