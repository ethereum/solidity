{
    let x := 115792089237316195423570985008687907853269984665640564039457584007913129639935
    let y := 1000
    // Cannot be removed, since it writes to bytes
    // 2**256 - 1, 0, 1, ..., 30
    mstore(x, y)

    // reads from [0, 32)
    pop(mload(0))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 115792089237316195423570985008687907853269984665640564039457584007913129639935
//     let y := 1000
//     mstore(x, y)
//     pop(mload(0))
// }
