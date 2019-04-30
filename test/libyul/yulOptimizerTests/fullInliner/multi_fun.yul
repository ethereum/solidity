{
	function f(a) -> x { x := add(a, a) }
	function g(b, c) -> y { y := mul(mload(c), f(b)) }
	let y := g(f(3), 7)
}
// ====
// step: fullInliner
// ----
// {
//     {
//         let _1 := 7
//         let a_6 := 3
//         let x_7 := 0
//         x_7 := add(a_6, a_6)
//         let b_8 := x_7
//         let c_9 := _1
//         let y_10 := 0
//         y_10 := mul(mload(c_9), f(b_8))
//         let y_1 := y_10
//     }
//     function f(a) -> x
//     {
//         x := add(a, a)
//     }
//     function g(b, c) -> y
//     {
//         let a_13 := b
//         let x_14 := 0
//         x_14 := add(a_13, a_13)
//         y := mul(mload(c), x_14)
//     }
// }
