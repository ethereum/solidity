object "obj" {
    code {
        let x
        x := 1
        setimmutable(0, "foo", 0)
        x := 2
    }
}
// "foo" was evaluated before `setimmutable`
// Expectation for this test should be changed to `ImpureBuiltinEncountered`
// if in the future if "foo" is evaluated after

// ----
// Execution result: UnlimitedLiteralEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
