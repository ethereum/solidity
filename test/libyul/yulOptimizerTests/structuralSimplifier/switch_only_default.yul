{
	switch mload(0) default { mstore(1, 2) }
}
// ====
// step: structuralSimplifier
// ----
// {
//     pop(mload(0))
//     {
//         mstore(1, 2)
//     }
// }
