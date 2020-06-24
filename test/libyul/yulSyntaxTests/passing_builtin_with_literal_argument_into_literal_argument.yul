{
    setimmutable(loadimmutable("abc"), "abc")
}
// ====
// dialect: evm
// ----
// TypeError 9114: (6-18): Function expects direct literals as arguments.
