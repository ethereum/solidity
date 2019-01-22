{
    let a := mload(0)
    if mload(1)
    {
        a := mload(1)
        a := mload(2)
        a := mload(3)
    }
    mstore(a, 0)
}
// ----
// ssaAndBack
// {
//     let a := mload(0)
//     if mload(1)
//     {
//         pop(mload(1))
//         pop(mload(2))
//         a := mload(3)
//     }
//     mstore(a, 0)
// }
