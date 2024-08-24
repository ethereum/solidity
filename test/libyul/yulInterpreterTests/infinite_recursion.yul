{
  function f() {
    f()
  }
  f()
}
// ----
// Trace:
//   Interpreter recursion depth exceeded
// Memory dump:
// Storage dump:
// Transient storage dump:
