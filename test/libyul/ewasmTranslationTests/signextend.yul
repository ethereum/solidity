{
  sstore(0, signextend(0, 0x86))
  sstore(1, signextend(0, 0x76))
  sstore(2, signextend(32, not(0)))
  sstore(3, signextend(5, 0xff8844553322))
}
// ----
// Trace:
//   INVALID()
// Memory dump:
// Storage dump:
