// An earlier version of the inliner produced
// pop(...) statements and explicitly removed them.
// This used to test that they are removed.
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
//         let f_x := 0
//         let f_r := mul(f_a, f_a)
//         f_x := add(f_r, f_r)
//         pop(add(f_x, _1))
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
