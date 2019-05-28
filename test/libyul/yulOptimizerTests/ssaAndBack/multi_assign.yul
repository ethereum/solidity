{
    let a := mload(0)
    a := mload(1)
    a := mload(2)
    a := mload(3)
    a := mload(4)
    mstore(a, 0)
}
// ====
// step: ssaAndBack
// ----
// {
//     let a_5 := mload(4)
//     mstore(a_5, 0)
// }
