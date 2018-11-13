{
    let a := sub(calldataload(1), calldataload(0))
    mstore(0, a)
}
// ----
// fullSimplify
// {
//     let _1 := 0
//     mstore(_1, sub(calldataload(1), calldataload(_1)))
// }
