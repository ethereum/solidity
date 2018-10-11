{
	let x := mul(f(0, mload(7)), 3)
	function f(a, b) -> c {
		c := mul(a, mload(add(b, c)))
	}
	sstore(x, f(mload(2), mload(2)))
}
// ----
// expressionSplitter
// {
//     let _1 := mload(7)
//     let _2 := f(0, _1)
//     let x := mul(_2, 3)
//     function f(a, b) -> c
//     {
//         let _3 := add(b, c)
//         let _4 := mload(_3)
//         c := mul(a, _4)
//     }
//     let _5 := mload(2)
//     let _6 := mload(2)
//     let _7 := f(_6, _5)
//     sstore(x, _7)
// }
