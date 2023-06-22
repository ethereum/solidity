{
	let x := mul(f(0, mload(7)), 3)
	function f(a, b) -> c {
		c := mul(a, mload(add(b, c)))
	}
	sstore(x, f(mload(2), mload(2)))
}
// ----
// step: expressionSplitter
//
// {
//     let _1 := 3
//     let _2 := 7
//     let _3 := mload(_2)
//     let _4 := f(0, _3)
//     let x := mul(_4, _1)
//     function f(a, b) -> c
//     {
//         let _5 := add(b, c)
//         let _6 := mload(_5)
//         c := mul(a, _6)
//     }
//     let _7 := 2
//     let _8 := mload(_7)
//     let _9 := 2
//     let _10 := mload(_9)
//     let _11 := f(_10, _8)
//     sstore(x, _11)
// }
