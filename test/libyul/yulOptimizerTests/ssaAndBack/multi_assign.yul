{
    let a := mload(0)
    a := mload(1)
    a := mload(2)
    a := mload(3)
    a := mload(4)
    mstore(a, 0)
}
// ----
// ssaAndBack
// {
//     pop(mload(0))
//     pop(mload(1))
//     pop(mload(2))
//     pop(mload(3))
//     let a_5 := mload(4)
//     mstore(a_5, 0)
// }
