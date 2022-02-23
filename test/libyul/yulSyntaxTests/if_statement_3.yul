{
	function f() -> x:bool {}

	if f()
	{
		let b:bool := f()
	}
}
// ====
// dialect: evmTyped
// ----
