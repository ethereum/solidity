{
    let a
    let b
    let x
    if a {
        if b {
            x := 2
        }
    }
    // Should create new SSA variables for x here,
    // but not above because end of block
    mstore(0, x)
}
// ====
// step: ssaTransform
// ----
// {
//     let a
//     let b
//     let x_1
//     let x := x_1
//     if a
//     {
//         if b
//         {
//             let x_2 := 2
//             x := x_2
//         }
//     }
//     let x_3 := x
//     mstore(0, x_3)
// }
