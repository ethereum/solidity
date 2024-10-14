{
    let x
    x := 1
    mcopy(0, 0, 0)
    x := 2
}
// ====
// EVMVersion: >=cancun
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
