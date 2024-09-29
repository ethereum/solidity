{
    let x
    x := 1
    selfdestruct(0)
    x := 2
}
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
