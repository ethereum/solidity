using {
    f as ++,
    f as --,
    f as x,
    f as delete,
    f as new,
    f as ()
} for int256;

// ----
// ParserError 4403: (17-19): The operator ++ cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~, <<, >>, **, !
// ParserError 4403: (30-32): The operator -- cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~, <<, >>, **, !
// ParserError 4403: (43-44): The operator cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~, <<, >>, **, !
// ParserError 4403: (55-61): The operator delete cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~, <<, >>, **, !
// ParserError 4403: (72-75): The operator new cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~, <<, >>, **, !
// ParserError 4403: (86-87): The operator ( cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~, <<, >>, **, !
// ParserError 2314: (87-88): Expected '}' but got ')'
