contract C {
    uint[uint(1)] valid_size_valid_expr1;
    uint[uint(2**256-1)] valid_size_valid_expr2;
    uint[uint(2**256)] invalid_size_invalid_expr3;

    uint[int(1)] valid_size_invalid_expr4;
    uint[int(2**256-1)] valid_size_invalid_expr5;
    uint[int(2**256)] invalid_size_invalid_expr6;
}
// ----
// TypeError 5462: (113-125): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (165-171): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (208-221): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (258-269): Invalid array length, expected integer literal or constant expression.
