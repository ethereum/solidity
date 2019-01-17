{
    let a := mload(0)
    let b := mload(a)
    a := mload(b)
    b := mload(a)
    a := mload(b)
    b := mload(a)
    mstore(a, b)
}
// ----
// ssaAndBack
// {
//     let a_1 := mload(0)
//     let b_2 := mload(a_1)
//     let a_3 := mload(b_2)
//     let b_4 := mload(a_3)
//     let a_5 := mload(b_4)
//     let b_6 := mload(a_5)
//     mstore(a_5, b_6)
// }
