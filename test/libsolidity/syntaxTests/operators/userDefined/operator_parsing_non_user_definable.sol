using {
    f as ++,
    f as --,
    f as x,
    f as delete,
    f as new,
    f as ()
} for int256;
// ----
// ParserError 4403: (17-19): Not a user-definable operator: ++. Only the following operators can be user-defined: |, &, ^, ~, <<, >>, +, -, *, /, %, **, ==, !=, <, >, <=, >=, !
// ParserError 4403: (30-32): Not a user-definable operator: --. Only the following operators can be user-defined: |, &, ^, ~, <<, >>, +, -, *, /, %, **, ==, !=, <, >, <=, >=, !
// ParserError 4403: (43-44): Not a user-definable operator: x. Only the following operators can be user-defined: |, &, ^, ~, <<, >>, +, -, *, /, %, **, ==, !=, <, >, <=, >=, !
// ParserError 4403: (55-61): Not a user-definable operator: delete. Only the following operators can be user-defined: |, &, ^, ~, <<, >>, +, -, *, /, %, **, ==, !=, <, >, <=, >=, !
// ParserError 4403: (72-75): Not a user-definable operator: new. Only the following operators can be user-defined: |, &, ^, ~, <<, >>, +, -, *, /, %, **, ==, !=, <, >, <=, >=, !
// ParserError 4403: (86-87): Not a user-definable operator: (. Only the following operators can be user-defined: |, &, ^, ~, <<, >>, +, -, *, /, %, **, ==, !=, <, >, <=, >=, !
// ParserError 2314: (87-88): Expected '}' but got ')'
