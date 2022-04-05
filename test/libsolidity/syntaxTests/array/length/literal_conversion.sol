contract C {
    uint[uint(1)] valid_size_invalid_expr1;
    uint[uint(2**256-1)] valid_size_invalid_expr2;
    uint[uint(2**256)] invalid_size_invalid_expr3;

    uint[int(1)] valid_size_invalid_expr4;
    uint[int(2**256-1)] valid_size_invalid_expr5;
    uint[int(2**256)] invalid_size_invalid_expr6;
}
// ----
// TypeError 5462: (22-29): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (66-80): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (117-129): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (169-175): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (212-225): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (262-273): Invalid array length, expected integer literal or constant expression.
