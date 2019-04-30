{
	// The component will remove the empty block after
	// it has handled the outer block.
	// The idea behind this test is that the component
	// does not perform replacements across blocks because
	// they usually have contents, but adding contents
	// will reduce the scope of the test.
	let a := mload(2)
	let x := calldataload(a)
	{
	}
	sstore(x, 3)
}
// ====
// step: expressionJoiner
// ----
// {
//     let x := calldataload(mload(2))
//     sstore(x, 3)
// }
