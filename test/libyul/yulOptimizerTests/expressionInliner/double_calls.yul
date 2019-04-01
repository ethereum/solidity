{
	function f(a) -> x { x := add(a, a) }
	function g(b, c) -> y { y := mul(mload(c), f(b)) }
	let y := g(calldatasize(), 7)
}
// ====
// step: expressionInliner
// ----
// {
//     function f(a) -> x
//     {
//         x := add(a, a)
//     }
//     function g(b, c) -> y
//     {
//         y := mul(mload(c), add(b, b))
//     }
//     let y_1 := mul(mload(7), add(calldatasize(), calldatasize()))
// }
