{
    let x
    x := 1
    pop(selfbalance())
    x := 2
}
// ====
// EVMVersion: >=istanbul
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
