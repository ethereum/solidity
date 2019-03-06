{
	let y := 200
	switch add(y, 4)
	case 0 { }
	case 1 { }
	default { }

	switch mload(4)
	case 0 { }
	case 1 { }
	default { }
}
// ----
// structuralSimplifier
// {
//     let y := 200
//     pop(add(y, 4))
//     pop(mload(4))
// }
