{
    function f() {
        mstore(0, 5)
        if calldataload(0) { leave }
        mstore(0x20, 5)
        revert(0, 0)
    }
    f()
}
// ----
// step: unusedStoreEliminator
//
// {
//     { f() }
//     function f()
//     {
//         mstore(0, 5)
//         if calldataload(0) { leave }
//         let _1 := 5
//         let _2 := 0x20
//         revert(0, 0)
//     }
// }
