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
// step: fullInliner
//
// {
//     {
//         let _1 := 2
//         let a_1 := 7
//         let x_1 := 0
//         let r_1 := mul(a_1, a_1)
//         x_1 := add(r_1, r_1)
//         pop(add(x_1, _1))
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
