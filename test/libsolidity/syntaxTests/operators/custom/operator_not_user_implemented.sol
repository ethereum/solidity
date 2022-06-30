using {
    shiftL as <<, shiftR as >>,
    exp as **, neg as !
} for int256;

// ----
// ParserError 1885: (22-24): The operator << cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 1885: (36-38): The operator >> cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 1885: (51-53): The operator ** cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 1885: (62-63): The operator ! cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
