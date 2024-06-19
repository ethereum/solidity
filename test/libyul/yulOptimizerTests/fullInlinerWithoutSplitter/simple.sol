{
    function f(a) -> x {
        x := add(a, a)
    }
    let y := f(2)
}
// ----
// step: fullInlinerWithoutSplitter
//
// {
//     {
//         let a_1 := 2
//         let x_1 := 0
//         x_1 := add(a_1, a_1)
//         let y := x_1
//     }
//     function f(a) -> x
//     { x := add(a, a) }
// }
