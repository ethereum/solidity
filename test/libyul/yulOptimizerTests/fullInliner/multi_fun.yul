{
	function f(a) -> x { x := add(a, a) }
	function g(b, c) -> y { y := mul(mload(c), f(b)) }
	let y := g(f(3), 7)
}
// ----
// step: fullInliner
//
// {
//     {
//         let _1 := 7
//         let a_2 := 3
//         let x_2 := 0
//         x_2 := add(a_2, a_2)
//         let _2 := x_2
//         let c_1 := _1
//         let b_1 := _2
//         let y_2 := 0
//         let a_3 := b_1
//         let x_3 := 0
//         x_3 := add(a_3, a_3)
//         y_2 := mul(mload(c_1), x_3)
//         let y_1 := y_2
//     }
//     function f(a) -> x
//     { x := add(a, a) }
//     function g(b, c) -> y
//     {
//         let a_1 := b
//         let x_1 := 0
//         x_1 := add(a_1, a_1)
//         y := mul(mload(c), x_1)
//     }
// }
