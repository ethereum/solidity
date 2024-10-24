{
    let double := verbatim_1i_1o(hex"600202", 1)
}
// The hext sequence was evaluated before verbatim
// Expectation for this test should be changed to `ImpureBuiltinEncountered`
// if in the future if the hex sequence is evaluated after.

// ----
// Execution result: UnlimitedLiteralEncountered
// Outer most variable values:
//
// Call trace:
