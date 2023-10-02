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
//         let a_8 := 3
//         let x_9 := 0
//         x_9 := add(a_8, a_8)
//         let _3 := x_9
//         let c_10 := _1
//         let b_11 := _3
//         let y_12 := 0
//         let a_6_13 := b_11
//         let x_7_14 := 0
//         x_7_14 := add(a_6_13, a_6_13)
//         y_12 := mul(mload(c_10), x_7_14)
//         let y_1 := y_12
//     }
//     function f(a) -> x
//     { x := add(a, a) }
//     function g(b, c) -> y
//     {
//         let a_6 := b
//         let x_7 := 0
//         x_7 := add(a_6, a_6)
//         y := mul(mload(c), x_7)
//     }
// }
