{
    let x := calldataload(0)
    for {} x { x := add(x, 1) } {
        //if lt(x, 0x42) { break }
        sstore(x, 0x21)
    }
    sstore(0x10, 0x10)
}
// ====
// stackOptimization: true
// ----
//   stop
