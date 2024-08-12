{
	let a:u256 function f() {}
}
// ====
// dialect: evmTyped
// ----
// step: functionGrouper
//
// {
//     { let a }
//     function f()
//     { }
// }
