{
  extcodecopy(address(), 0x100, 0, extcodesize(address()))
  sstore(0, mload(0x100))
}
// ----
// Trace:
// Memory dump:
// Storage dump:
