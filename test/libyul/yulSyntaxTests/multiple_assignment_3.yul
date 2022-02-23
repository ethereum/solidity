{
	function f(a:u256) -> r1:u256, r2:u256 {
		r1 := a
		r2 := 7:u256
	}
	let x:u256 := 9:u256
	let y:u256 := 2:u256
	x, y := f(x)
}
// ====
// dialect: evmTyped
// ----
