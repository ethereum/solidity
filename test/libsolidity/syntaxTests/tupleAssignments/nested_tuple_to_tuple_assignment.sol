contract C {
    bool x;

    function foo() external {
        (((),x)&x) = ((), true);
        (((),x)+x) = ((), true);
        (((),x)-x) = ((), true);
        (((),x)/x) = ((), true);
        (((),x)*x) = ((), true);
        (((),x)|x) = ((), true);
    }
}
// ----
// TypeError 6473: (66-68): Tuple component cannot be empty.
// TypeError 2271: (65-73): Built-in binary operator & cannot be applied to types tuple(tuple(),bool) and bool.
// TypeError 4247: (65-73): Expression has to be an lvalue.
// TypeError 6473: (78-80): Tuple component cannot be empty.
// TypeError 6473: (99-101): Tuple component cannot be empty.
// TypeError 2271: (98-106): Built-in binary operator + cannot be applied to types tuple(tuple(),bool) and bool.
// TypeError 4247: (98-106): Expression has to be an lvalue.
// TypeError 6473: (111-113): Tuple component cannot be empty.
// TypeError 6473: (132-134): Tuple component cannot be empty.
// TypeError 2271: (131-139): Built-in binary operator - cannot be applied to types tuple(tuple(),bool) and bool.
// TypeError 4247: (131-139): Expression has to be an lvalue.
// TypeError 6473: (144-146): Tuple component cannot be empty.
// TypeError 6473: (165-167): Tuple component cannot be empty.
// TypeError 2271: (164-172): Built-in binary operator / cannot be applied to types tuple(tuple(),bool) and bool.
// TypeError 4247: (164-172): Expression has to be an lvalue.
// TypeError 6473: (177-179): Tuple component cannot be empty.
// TypeError 6473: (198-200): Tuple component cannot be empty.
// TypeError 2271: (197-205): Built-in binary operator * cannot be applied to types tuple(tuple(),bool) and bool.
// TypeError 4247: (197-205): Expression has to be an lvalue.
// TypeError 6473: (210-212): Tuple component cannot be empty.
// TypeError 6473: (231-233): Tuple component cannot be empty.
// TypeError 2271: (230-238): Built-in binary operator | cannot be applied to types tuple(tuple(),bool) and bool.
// TypeError 4247: (230-238): Expression has to be an lvalue.
// TypeError 6473: (243-245): Tuple component cannot be empty.
