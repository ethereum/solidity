{
    let u160max := 0xffffffffffffffffffffffffffffffffffffffff
    let a := create2(0, u160max, 0, 0)
    let b := and(u160max, create2(0, u160max, 0, 0))
    mstore(0, eq(a, b))
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
//   CREATE2(0, 0, 0, 0)
//   CREATE2(0, 0, 0, 0)
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000001
// Storage dump:
