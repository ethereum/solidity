{
  mstore(0, exp(3,not(1)))
}
// ----
// Trace:
//   MSTORE_AT_SIZE(0, 32) [8e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e39]
// Memory dump:
//      0: 8e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e39
// Storage dump:
