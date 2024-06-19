{
	function f(a) -> x {
		let r := mul(a, a)
		x := add(r, r)
	}
	let y := add(f(sload(mload(2))), mload(7))
}
// ----
// step: fullInliner
//
// {
//     {
//         let _1 := mload(7)
//         let a_1 := sload(mload(2))
//         let x_1 := 0
//         let r_1 := mul(a_1, a_1)
//         x_1 := add(r_1, r_1)
//         let y := add(x_1, _1)
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
