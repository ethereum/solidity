// The calls to g and h cannot be moved because g and h are not movable. Therefore, the call
// to f is not inlined.
{
	function f(a, b) -> x { x := add(b, a) }
	function g() -> y { y := mload(0) mstore(0, 4) }
	function h() -> z { mstore(0, 4) z := mload(0) }
	let r := f(g(), h())
}
// ====
// step: expressionInliner
// ----
// {
//     function f(a, b) -> x
//     { x := add(b, a) }
//     function g() -> y
//     {
//         y := mload(0)
//         mstore(0, 4)
//     }
//     function h() -> z
//     {
//         mstore(0, 4)
//         z := mload(0)
//     }
//     let r := f(g(), h())
// }
