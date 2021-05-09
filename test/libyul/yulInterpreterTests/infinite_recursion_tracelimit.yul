{
  function f() {
    log0(0x0, 0x0)
    f()
  }
  f()
}
// ----
// Trace:
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   LOG0(0, 0)
//   Trace size limit reached.
// Memory dump:
// Storage dump:
