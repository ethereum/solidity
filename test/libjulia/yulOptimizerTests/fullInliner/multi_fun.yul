{
	function f(a) -> x { x := add(a, a) }
	function g(b, c) -> y { y := mul(mload(c), f(b)) }
	let y := g(f(3), 7)
}
// ----
// fullInliner
// {
//     {
//         let g_c := 7
//         let f_a_1 := 3
//         let f_x_1
//         {
//             f_x_1 := add(f_a_1, f_a_1)
//         }
//         let g_y
//         {
//             let g_f_a := f_x_1
//             let g_f_x
//             {
//                 g_f_x := add(g_f_a, g_f_a)
//             }
//             g_y := mul(mload(g_c), g_f_x)
//         }
//         let y_1 := g_y
//     }
//     function f(a) -> x
//     {
//         x := add(a, a)
//     }
//     function g(b, c) -> y
//     {
//         let f_a := b
//         let f_x
//         {
//             f_x := add(f_a, f_a)
//         }
//         y := mul(mload(c), f_x)
//     }
// }
