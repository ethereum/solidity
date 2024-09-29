{
    let x
    x := 1
    pop(gaslimit())
    x := 2
}
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
