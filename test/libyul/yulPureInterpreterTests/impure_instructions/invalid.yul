{
    let x
    x := 1
    invalid()
    x := 2
}
// ----
// Execution result: ImpureBuiltinEncountered
// Outter most variable values:
//   x = 1
//
// Call trace:
