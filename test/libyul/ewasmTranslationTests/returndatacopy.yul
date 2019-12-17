{
  returndatacopy(0x100, 0, returndatasize())
  sstore(0, mload(0x100))
}
// ====
// EVMVersion: >=byzantium
// ----
// Trace:
// Memory dump:
// Storage dump:
