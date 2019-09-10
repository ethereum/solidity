{
	let a:u256 function f() {}
}
// ====
// step: functionGrouper
// yul: true
// ----
// {
//     { let a:u256 }
//     function f()
//     { }
// }
