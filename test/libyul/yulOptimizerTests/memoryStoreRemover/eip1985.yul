{
    // 2**128
    let x := 340282366920938463463374607431768211456
    // The optimization step will remove the following `mstore`.
    // EIP 1985 limits `msize()` to `2**32` and the `mstore`
    // beyond the region would result in out of gas.
    // By removing `mstore`, we are converting a snippet
    // that will be always OOG into one that would execute.
    mstore(x, 100)
    sstore(0, 10)
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 340282366920938463463374607431768211456
//     pop(100)
//     sstore(0, 10)
// }
