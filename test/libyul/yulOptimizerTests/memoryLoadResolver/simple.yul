{
    let key := 64
    let value := 128
    mstore(key, value)

    // should be replaced by sstore(0, value)
    sstore(0, mload(key))
}
// ----
// step: memoryLoadResolver
//
// {
//     let key := 64
//     let value := 128
//     mstore(key, value)
//     sstore(0, value)
// }
