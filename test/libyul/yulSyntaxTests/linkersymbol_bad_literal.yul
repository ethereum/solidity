{
    pop(linkersymbol(0))
    pop(linkersymbol(true))
    pop(linkersymbol(false))
}
// ====
// dialect: evm
// ----
// TypeError 5859: (23-24='0'): Function expects string literal.
// TypeError 5859: (48-52='true'): Function expects string literal.
// TypeError 5859: (76-81='false'): Function expects string literal.
