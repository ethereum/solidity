{
    let a := mload(0)
    if mload(1)
    {
        a := mload(1)
    }
    mstore(a, 0)
}
// ----
// ssaAndBack
// {
//     let a := mload(0)
//     if mload(1)
//     {
//         a := mload(1)
//     }
//     mstore(a, 0)
// }
