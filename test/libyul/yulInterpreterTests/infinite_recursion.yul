{
  function f() {
    f()
  }
  f()
}
// ----
// Trace:
//   Interpreter execution step limit reached.
// Memory dump:
// Storage dump:
