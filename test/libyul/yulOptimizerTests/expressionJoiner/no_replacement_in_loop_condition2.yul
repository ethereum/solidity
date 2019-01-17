{
	let a := mload(0)
	for { } a {} {}
}
// ----
// expressionJoiner
// {
//     let a := mload(0)
//     for {
//     }
//     a
//     {
//     }
//     {
//     }
// }
