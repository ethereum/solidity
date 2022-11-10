contract C
{
    function f() public
    {
        int x = +(0, 0);
        int y = -(0, 0);
        (int z) = ~(0, 0);
        (int t) = !(0, 0);
    }
}
// ----
// SyntaxError 9636: (59-66): Use of unary + is disallowed.
// TypeError 4907: (59-66): Built-in unary operator + cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (51-66): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError 4907: (84-91): Built-in unary operator - cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (76-91): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError 4907: (111-118): Built-in unary operator ~ cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (101-118): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError 4907: (138-145): Built-in unary operator ! cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (128-145): Different number of components on the left hand side (1) than on the right hand side (2).
