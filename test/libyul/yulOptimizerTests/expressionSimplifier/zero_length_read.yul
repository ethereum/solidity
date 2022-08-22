{
	revert(calldataload(0), 0)
	revert(call(0,0,0,0,0,0,0), 0)
	calldatacopy(calldataload(1), calldataload(2), 0)
	return(calldataload(3), 0)
	codecopy(calldataload(4), calldataload(5), sub(42,42))
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0
//         revert(0, _1)
//         pop(call(_1, _1, _1, _1, _1, _1, _1))
//         revert(0, _1)
//         calldatacopy(0, calldataload(2), _1)
//         return(0, _1)
//         codecopy(0, calldataload(5), 0)
//     }
// }
