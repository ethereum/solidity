{
    let x
    x := 1
    pop(prevrandao())
    x := 2
}
// ====
// EVMVersion: >=paris
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
