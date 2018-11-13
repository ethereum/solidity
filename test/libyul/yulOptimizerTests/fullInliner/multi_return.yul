{
	function f(a) -> x, y {
		x := mul(a, a)
		y := add(a, x)
	}
	let r, s := f(mload(0))
	mstore(r, s)
}
// ----
// fullInliner
// {
//     {
//         let f_a := mload(0)
//         let f_x
//         let f_y
//         f_x := mul(f_a, f_a)
//         f_y := add(f_a, f_x)
//         let r := f_x
//         mstore(r, f_y)
//     }
//     function f(a) -> x, y
//     {
//         x := mul(a, a)
//         y := add(a, x)
//     }
// }
