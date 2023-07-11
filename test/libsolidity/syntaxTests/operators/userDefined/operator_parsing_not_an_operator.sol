using {
    f as x,
    f as operator,
    f as as,
    f as 123,
    f as ()
} for int256 global;
// ----
// ParserError 4403: (17-18): Not a user-definable operator: x. Only the following operators can be user-defined: |, &, ^, ~, +, -, *, /, %, ==, !=, <, >, <=, >=
// ParserError 4403: (29-37): Not a user-definable operator: operator. Only the following operators can be user-defined: |, &, ^, ~, +, -, *, /, %, ==, !=, <, >, <=, >=
// ParserError 4403: (48-50): Not a user-definable operator: as. Only the following operators can be user-defined: |, &, ^, ~, +, -, *, /, %, ==, !=, <, >, <=, >=
// ParserError 4403: (61-64): Not a user-definable operator: 123. Only the following operators can be user-defined: |, &, ^, ~, +, -, *, /, %, ==, !=, <, >, <=, >=
// ParserError 4403: (75-76): Not a user-definable operator: (. Only the following operators can be user-defined: |, &, ^, ~, +, -, *, /, %, ==, !=, <, >, <=, >=
// ParserError 2314: (76-77): Expected '}' but got ')'
