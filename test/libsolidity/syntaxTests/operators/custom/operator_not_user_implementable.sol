using {
    shiftL as <<, shiftR as >>,
    exp as **, neg as !
} for int256;

// ----
// ParserError 4403: (22-24): The operator << cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (36-38): The operator >> cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (51-53): The operator ** cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (62-63): The operator ! cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
