{
    let x := 0
    let y := 100
    // Actually cannot be removed.
    mstore(x, y)

    // Even though this cannot happen in practice,
    // we'll unfortunately have to deal with this situation.
    // reads from bytes 2*256 - 1, 0, ..., 30
    pop(mload(115792089237316195423570985008687907853269984665640564039457584007913129639935))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 100
//     mstore(x, y)
//     pop(mload(115792089237316195423570985008687907853269984665640564039457584007913129639935))
// }
