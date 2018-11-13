{
    let a := 1
    a := 2
    a := 3
    a := 4
    mstore(0, a)
}
// ----
// ssaPlusCleanup
// {
//     let a_1 := 1
//     let a := a_1
//     let a_2 := 2
//     let a_3 := 3
//     let a_4 := 4
//     mstore(0, a_4)
// }
