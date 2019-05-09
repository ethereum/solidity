{
	switch mload(0) default { mstore(1, 2) }
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     pop(mload(0))
//     { mstore(1, 2) }
// }
