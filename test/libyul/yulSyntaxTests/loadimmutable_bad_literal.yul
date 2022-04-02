{
    pop(loadimmutable(0))
    pop(loadimmutable(true))
    pop(loadimmutable(false))
}
// ====
// dialect: evm
// ----
// TypeError 5859: (24-25='0'): Function expects string literal.
// TypeError 5859: (50-54='true'): Function expects string literal.
// TypeError 5859: (79-84='false'): Function expects string literal.
