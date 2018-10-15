{ let a := add(7, sub(mload(0), 7)) }
// ----
// expressionSimplifier
// {
//     let a := mload(0)
// }
