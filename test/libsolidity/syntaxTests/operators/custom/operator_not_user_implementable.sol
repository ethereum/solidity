using {
    f as <<,
    f as >>,
    f as **,
    f as ++,
    f as !,
    f as x,
    f as delete,
    f as new,
    f as ()
} for int256;

// ----
// ParserError 4403: (17-19): The operator << cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (30-32): The operator >> cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (43-45): The operator ** cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (56-58): The operator ++ cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (69-70): The operator ! cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (81-82): The operator cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (93-99): The operator delete cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (110-113): The operator new cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (124-125): The operator ( cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 2314: (125-126): Expected '}' but got ')'
