{
	function f(a) -> x {
		let r := mul(a, a)
		x := add(r, r)
	}
	let y := add(f(sload(mload(2))), mload(7))
}
// ----
// fullInliner
// {
//     {
//         let _2 := mload(7)
//         let a_7 := sload(mload(2))
//         let x_8 := 0
//         let r_9 := mul(a_7, a_7)
//         x_8 := add(r_9, r_9)
//         let y := add(x_8, _2)
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
