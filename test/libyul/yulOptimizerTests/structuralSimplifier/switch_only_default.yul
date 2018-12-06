{
	switch mload(0) default { mstore(1, 2) }
}
// ----
// structuralSimplifier
// {
//     pop(mload(0))
//     {
//         mstore(1, 2)
//     }
// }
