function f() pure {
    assembly " evmasm" {}
}
// ----
// ParserError 4531: (33-42): Only "evmasm" supported.
