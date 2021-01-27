{
    let key := 64
    let value := 128
    mstore(key, value)

    if callvalue() { revert(0, 0) }

    let _1 := 0
    let c := calldatasize()

    // Does not invalidate location 64
    calldatacopy(value, _1, c)

    // Should be replaced by out := value
    let out := mload(key)
    sstore(0, out)
}
// ----
// step: memoryLoadResolver
//
// {
//     let key := 64
//     let value := 128
//     mstore(key, value)
//     if callvalue() { revert(0, 0) }
//     let _1 := 0
//     let c := calldatasize()
//     calldatacopy(value, _1, c)
//     let out := value
//     sstore(0, out)
// }
