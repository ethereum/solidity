{
	let x := mul(f(0, mload(7)), 3)
	function f(a, b) -> c {
		c := mul(a, mload(add(b, c)))
	}
	sstore(x, f(mload(2), mload(2)))
}
// ====
// step: expressionSplitter
// ----
// {
//     let _1 := 3
//     let _2 := 7
//     let _3 := mload(_2)
//     let _4 := 0
//     let _5 := f(_4, _3)
//     let x := mul(_5, _1)
//     function f(a, b) -> c
//     {
//         let _6 := add(b, c)
//         let _7 := mload(_6)
//         c := mul(a, _7)
//     }
//     let _8 := 2
//     let _9 := mload(_8)
//     let _10 := 2
//     let _11 := mload(_10)
//     let _12 := f(_11, _9)
//     sstore(x, _12)
// }
