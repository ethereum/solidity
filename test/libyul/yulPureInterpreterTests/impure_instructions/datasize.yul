object "obj" {
    code {
        let x
        x := 1
        pop(datasize("obj"))
        x := 2
    }
}
// "obj" was evaluated before `datasize`
// Expection for this test should be changed to `ImpureBuiltinEncountered`
// if in the future if "obj" is evaluated after

// ----
// Execution result: UnlimitedLiteralEncountered
// Outter most variable values:
//   x = 1
//
// Call trace:
