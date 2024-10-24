{
    let i
    for { i := 0 } lt(i, 100) { i := add(i, 1) }
    {
    }
}
// ====
// maxSteps: 200
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   i = 100
//
// Call trace:
