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
// Outter most variable values:
//
// Call trace:
