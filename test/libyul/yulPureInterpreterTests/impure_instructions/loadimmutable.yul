object "obj" {
    code {
        let x
        x := 1
        pop(loadimmutable("foo"))
        x := 2
    }
}
// "foo" was evaluated before `loadimmutable`
// Expection for this test should be changed to `ImpureBuiltinEncountered`
// if in the future if "foo" is evaluated after

// ----
// Execution result: UnlimitedLiteralEncountered
// Outter most variable values:
//   x = 1
//
// Call trace:
