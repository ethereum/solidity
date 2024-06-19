{
	function f(a: u256) -> x: bool, y:u256 {
		y := mul(a, a)
	}
	let r: bool, s: u256 := f(mload(3))
}
// ====
// dialect: evmTyped
// ----
// step: fullInliner
//
// {
//     {
//         let a_1 := mload(3)
//         let x_1:bool := false
//         let y_1 := 0
//         y_1 := mul(a_1, a_1)
//         let r:bool := x_1
//         let s := y_1
//     }
//     function f(a) -> x:bool, y
//     { y := mul(a, a) }
// }
