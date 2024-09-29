{
    let x
    x := 1
    pop(blobbasefee())
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
