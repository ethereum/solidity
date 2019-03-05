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
//         let a_5 := 7
//         let x_6 := 0
//         let r_7 := mul(a_5, a_5)
//         x_6 := add(r_7, r_7)
//         pop(add(x_6, _1))
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
