contract C
{
    function f() public
    {
        int y = -(0, 0);
        (int z) = ~(0, 0);
        (int t) = !(0, 0);
    }
}
// ----
// TypeError 4907: (59-66): Built-in unary operator - cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (51-66): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError 4907: (86-93): Built-in unary operator ~ cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (76-93): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError 4907: (113-120): Built-in unary operator ! cannot be applied to type tuple(int_const 0,int_const 0).
// TypeError 7364: (103-120): Different number of components on the left hand side (1) than on the right hand side (2).
