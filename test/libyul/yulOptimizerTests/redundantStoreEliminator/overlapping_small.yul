{
    let _1 := 0
    let _2 := callvalue()
    let _3 := 0x4e487b71
    let _4 := 224
    let _5 := 7
    mstore(_1, _5)
    let _6 := 0x32
    let _7 := 4
    mstore(_7, _6)
    let _8 := 0x24
    revert(_1, _8)
}
// ----
// step: redundantStoreEliminator
//
// {
//     let _1 := 0
//     let _2 := callvalue()
//     let _3 := 0x4e487b71
//     let _4 := 224
//     mstore(_1, 7)
//     mstore(4, 0x32)
//     revert(_1, 0x24)
// }
