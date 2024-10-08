{
    function f() {
        f()
    }
    f()
}
// ====
// maxTraceSize: 0
// ----
// Execution result: RecursionDepthLimitReached
// Outer most variable values:
//
// Call trace:
