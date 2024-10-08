{
    function f(x) -> y
    {
        if lt(x, 150)
        {
            y := f(add(x, 1))
        }
        if eq(x, 150)
        {
            y := x
        }
    }
    let res := f(0)
}

// ====
// maxRecursionDepth: 160
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   res = 150
//
// Call trace:
