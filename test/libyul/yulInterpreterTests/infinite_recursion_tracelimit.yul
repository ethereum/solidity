{
  function f() {
    log0(0x0, 0x0)
    f()
  }
  f()
}
// ----
// Trace:
//   Interpreter execution step limit reached.
// Memory dump:
// Storage dump:
