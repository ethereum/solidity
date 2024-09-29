{
    let x
    x := 1
    pop(chainid())
    x := 2
}
// ----
// Execution result: ImpureBuiltinEncountered
// Outter most variable values:
//   x = 1
//
// Call trace:
