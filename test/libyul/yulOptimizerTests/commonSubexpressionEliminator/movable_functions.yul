{
    function double(x) -> y { y := add(x, x) }
    function double_with_se(x) -> y { y := add(x, x) mstore(40, 4) }
    let i := mload(3)
    let a := double(i)
    let b := double(i)
    let c := double_with_se(i)
    let d := double_with_se(i)
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     function double(x) -> y
//     { y := add(x, x) }
//     function double_with_se(x_1) -> y_2
//     {
//         y_2 := add(x_1, x_1)
//         mstore(40, 4)
//     }
//     let i := mload(3)
//     let a := double(i)
//     let b := a
//     let c := double_with_se(i)
//     let d := double_with_se(i)
// }
