{
    let x
    x := 1
    pop(staticcall(0, 0, 0, 0, 0, 0))
    x := 2
}
// ====
// EVMVersion: >=byzantium
// ----
// Execution result: ImpureBuiltinEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
