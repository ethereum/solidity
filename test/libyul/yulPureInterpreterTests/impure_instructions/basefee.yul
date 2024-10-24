{
    let x
    x := 1
    pop(basefee())
    x := 2
}
// ====
// EVMVersion: >=london
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
