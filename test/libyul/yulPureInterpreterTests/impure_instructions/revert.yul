{
    let x
    x := 1
    revert(0, 0)
    x := 2
}
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
