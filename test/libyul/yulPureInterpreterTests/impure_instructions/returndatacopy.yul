{
    let x
    x := 1
    returndatacopy(0, 0, 0)
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
