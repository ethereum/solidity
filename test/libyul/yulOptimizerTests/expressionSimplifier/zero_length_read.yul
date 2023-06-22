{
	revert(calldataload(0), 0)
	revert(call(0,0,0,0,0,0,0), 0)
	calldatacopy(calldataload(1), calldataload(2), 0)
	return(calldataload(3), 0)
	codecopy(calldataload(4), calldataload(5), sub(42,42))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         revert(0, 0)
//         pop(call(0, 0, 0, 0, 0, 0, 0))
//         revert(0, 0)
//         calldatacopy(0, calldataload(2), 0)
//         return(0, 0)
//         codecopy(0, calldataload(5), 0)
//     }
// }
