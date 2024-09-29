{
    let i
    for { i := 0 } lt(i, 100) { i := add(i, 1) }
    {
    }
}
// ====
// maxSteps: 100
// ----
// Execution result: StepLimitReached
// Outer most variable values:
//   i = 96
//
// Call trace:
