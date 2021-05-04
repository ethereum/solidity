// Test to see if the solver can prove that there is no overflow
// during memory calculations.
{
    let x := 0
    let y := 100
    // can be removed
    mstore(x, y)
    // reads from [500, 500 + calldatasize())
    // since `500 + calldatasize()` does not overflow, it does not read from [0, 32)
    // Assuming EIP 1985, the value of calldatasize is at most 2**32.
    pop(staticcall(10000, 0, 500, calldatasize(), 0, 0))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 100
//     pop(y)
//     pop(staticcall(10000, 0, 500, calldatasize(), 0, 0))
// }
