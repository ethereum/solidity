{
	function f(a) -> x { x := add(a, a) }
	function g(b, c) -> y { y := mul(mload(c), f(b)) }
	let y := g(f(3), 7)
}
// ----
// fullInliner
// {
//     {
//         let _1 := 7
//         let f_a := 3
//         let f_x
//         f_x := add(f_a, f_a)
//         let g_b := f_x
//         let g_c := _1
//         let g_y
//         g_y := mul(mload(g_c), f(g_b))
//         let y_1 := g_y
//     }
//     function f(a) -> x
//     {
//         x := add(a, a)
//     }
//     function g(b, c) -> y
//     {
//         let f_a_6 := b
//         let f_x_7
//         f_x_7 := add(f_a_6, f_a_6)
//         y := mul(mload(c), f_x_7)
//     }
// }
