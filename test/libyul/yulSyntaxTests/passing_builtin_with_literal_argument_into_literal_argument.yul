{
    setimmutable(0, loadimmutable("abc"), "abc")
}
// ====
// dialect: evm
// ----
// TypeError 9114: (6-18='setimmutable'): Function expects direct literals as arguments.
