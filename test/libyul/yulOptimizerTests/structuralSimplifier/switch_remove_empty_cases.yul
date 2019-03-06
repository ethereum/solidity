{
	let y := 200
	switch y
	case 0 { }
	case 1 { y := 9 }
	default { }
}
// ----
// structuralSimplifier
// {
//     let y := 200
//     if eq(1, y)
//     {
//         y := 9
//     }
// }
