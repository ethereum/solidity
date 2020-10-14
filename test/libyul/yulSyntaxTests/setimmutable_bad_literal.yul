{
    setimmutable(0, 0, 0x1234567890123456789012345678901234567890)
    setimmutable(0, true, 0x1234567890123456789012345678901234567890)
    setimmutable(0, false, 0x1234567890123456789012345678901234567890)
}
// ====
// dialect: evm
// ----
// TypeError 5859: (22-23): Function expects string literal.
// TypeError 5859: (89-93): Function expects string literal.
// TypeError 5859: (159-164): Function expects string literal.
