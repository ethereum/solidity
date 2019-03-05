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
//         let a_3 := mload(0)
//         let x_4 := 0
//         let y_5 := 0
//         x_4 := mul(a_3, a_3)
//         y_5 := add(a_3, x_4)
//         let r := x_4
//         mstore(r, y_5)
//     }
//     function f(a) -> x, y
//     {
//         x := mul(a, a)
//         y := add(a, x)
//     }
// }
