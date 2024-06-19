{
	function f() { let x_1 := 0 }
	function g() { let x_2 := 0 }
}
// ----
// step: varNameCleaner
//
// {
//     { }
//     function f()
//     { let x_1 := 0 }
//     function g()
//     { let x_2 := 0 }
// }
