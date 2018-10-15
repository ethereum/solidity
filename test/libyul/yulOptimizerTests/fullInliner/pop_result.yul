// This tests that `pop(r)` is removed.
{
	function f(a) -> x {
		let r := mul(a, a)
		x := add(r, r)
	}
	pop(add(f(7), 2))
}
// ----
// fullInliner
// {
//     {
//         let _1 := 2
//         let f_a := 7
//         let f_x
//         {
//             let f_r := mul(f_a, f_a)
//             f_x := add(f_r, f_r)
//         }
//         {
//         }
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
