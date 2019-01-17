{
    let a := 1
    a := add(a, 2)
    a := add(a, 3)
    a := mload(add(a, 4))
    mstore(0, a)
}
// ----
// ssaPlusCleanup
// {
//     let a_1 := 1
//     let a := a_1
//     let a_2 := add(a_1, 2)
//     let a_3 := add(a_2, 3)
//     let a_4 := mload(add(a_3, 4))
//     mstore(0, a_4)
// }
