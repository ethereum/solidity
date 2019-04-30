{
    let a := mload(0)
    let b := mload(1)
    if mload(2) {
        a := mload(b)
        b := mload(a)
        a := mload(b)
        b := mload(a)
    }
    mstore(a, b)
}
// ====
// step: ssaAndBack
// ----
// {
//     let a := mload(0)
//     let b := mload(1)
//     if mload(2)
//     {
//         let a_3 := mload(b)
//         let b_4 := mload(a_3)
//         a := mload(b_4)
//         b := mload(a)
//     }
//     mstore(a, b)
// }
