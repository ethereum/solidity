// The full inliner currently does not work with
// functions returning multiple values.
{
	function f(a) -> x, y {
		x := mul(a, a)
		y := add(a, x)
	}
	let a, b := f(mload(0))
}
// ----
// fullInliner
// {
//     {
//         let a_1, b := f(mload(0))
//     }
//     function f(a) -> x, y
//     {
//         x := mul(a, a)
//         y := add(a, x)
//     }
// }
