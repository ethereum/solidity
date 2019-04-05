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
// step: fullSuite
// ----
// {
//     {
//         let a := mload(0)
//         let b := mload(1)
//         if mload(2)
//         {
//             a := mload(mload(mload(b)))
//             b := mload(a)
//         }
//         mstore(a, b)
//     }
// }
