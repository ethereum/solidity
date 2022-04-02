function f() pure {
    assembly " evmasm" {}
}
// ----
// ParserError 4531: (33-42='" evmasm"'): Only "evmasm" supported.
