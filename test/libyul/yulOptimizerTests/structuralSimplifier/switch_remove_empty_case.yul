{
	let y := 200
	switch y
	case 0 { }
	case 1 { y := 9 }
}
// ----
// structuralSimplifier
// {
//     let y := 200
//     switch y
//     case 1 {
//         y := 9
//     }
// }
