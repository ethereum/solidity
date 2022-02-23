{
  function f() -> x { mstore(0, 0x1337) }
  pop(byte(0, shr(0x8, f())))
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000001337
// Storage dump:
