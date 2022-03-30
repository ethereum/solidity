{
    let u160max := 0xffffffffffffffffffffffffffffffffffffffff
    let a := create(0, u160max, 0)
    let b := and(u160max, create(0, u160max, 0))
    mstore(0, eq(a, b))
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000001
// Storage dump:
