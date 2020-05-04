contract C
{
    function f() public
    {
        int x = delete (,0);
    }
}
// ----
// TypeError: (68-69): Expression has to be an lvalue.
// TypeError: (59-70): Unary operator delete cannot be applied to type tuple(,int_const 0)
