{
    let x := 0
    // y is not a SSA Variable
    let y := 20
    if calldataload(10) {
        y := 100
    }
    // The location is not read from.
    // Even though the value is not an identifier, let alone an SSA variable.
    mstore(x, calldataload(y))
    mstore(x, y)

    // does not read from memory
    pop(extcodesize(0))
    // does not read from [0, 32)
    log0(100, 200)
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 20
//     if calldataload(10) { y := 100 }
//     pop(calldataload(y))
//     pop(y)
//     pop(extcodesize(0))
//     log0(100, 200)
// }
