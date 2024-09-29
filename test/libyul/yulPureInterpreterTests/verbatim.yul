{
    let double := verbatim_1i_1o(hex"600202", 1)
}
// The hext sequence was evaluated before verbatim
// Expection for this test should be changed to `ImpureBuiltinEncountered`
// if in the future if the hex sequence is evaluated after.

// ----
// Execution result: UnlimitedLiteralEncountered
// Outter most variable values:
//
// Call trace:
