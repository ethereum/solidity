{
	switch 1 case 2 { mstore(0, 0) }
}
// ----
// structuralSimplifier
// {
//     if eq(2, 1)
//     {
//         mstore(0, 0)
//     }
// }
