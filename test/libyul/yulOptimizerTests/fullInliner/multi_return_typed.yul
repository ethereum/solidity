{
	function f(a: u256) -> x: bool, y:u256 {
		y := mul(a, a)
	}
	let r: bool, s: u256 := f(mload(3))
}
// ====
// dialect: evmTyped
// step: fullInliner
// ----
// {
//     {
//         let a_3 := mload(3)
//         let x_4:bool := false
//         let y_5 := 0
//         y_5 := mul(a_3, a_3)
//         let r:bool := x_4
//         let s := y_5
//     }
//     function f(a) -> x:bool, y
//     { y := mul(a, a) }
// }
