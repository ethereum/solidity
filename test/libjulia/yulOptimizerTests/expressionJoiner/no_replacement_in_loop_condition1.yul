{
	for { let b := mload(1) } b {} {}
}
// ----
// expressionJoiner
// {
//     for {
//         let b := mload(1)
//     }
//     b
//     {
//     }
//     {
//     }
// }
