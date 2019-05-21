object "main"
{
    code {
        datacopy(0, dataoffset("main"), datasize("main"))
        datacopy(32, dataoffset("sub"), datasize("sub"))
        sstore(0, mload(0))
        sstore(1, mload(32))
    }
    object "sub" { code { sstore(0, 1) } }
}
// ----
// Trace:
//   MSTORE_AT_SIZE(0, 2916)
//   MSTORE_AT_SIZE(32, 265)
//   MLOAD_FROM_SIZE(0, 32)
//   SSTORE(0, 0)
//   MLOAD_FROM_SIZE(32, 32)
//   SSTORE(1, 0)
// Memory dump:
// Storage dump:
