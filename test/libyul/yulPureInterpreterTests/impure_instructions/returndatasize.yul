{
    let x
    x := 1
    pop(returndatasize())
    x := 2
}
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
