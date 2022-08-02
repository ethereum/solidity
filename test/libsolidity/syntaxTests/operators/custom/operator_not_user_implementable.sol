using {
    shiftL as <<,
    shiftR as >>,
    exp as **,
    neg as !,
    f as x
} for int256;

// ----
// ParserError 4403: (22-24): The operator << cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (40-42): The operator >> cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (55-57): The operator ** cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (70-71): The operator ! cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
// ParserError 4403: (82-83): The operator cannot be user-implemented. This is only possible for the following operators: |, &, ^, +, -, *, /, %, ==, !=, <, >, <=, >=, ~
