{
    let x
    x := 1
    pop(create2(0, 0, 0, 0))
    x := 2
}
// ====
// EVMVersion: >=constantinople
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
