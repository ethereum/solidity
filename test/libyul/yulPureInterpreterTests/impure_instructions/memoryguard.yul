object "obj" {
    code {
        let x
        x := 1
        pop(memoryguard(0))
        x := 2
    }
}
// ----
// Execution result: UnlimitedLiteralEncountered
// Outer most variable values:
//   x = 1
//
// Call trace:
