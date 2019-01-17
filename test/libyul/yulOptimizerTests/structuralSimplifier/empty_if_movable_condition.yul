{ let a := mload(0) if a {} }
// ----
// structuralSimplifier
// {
//     let a := mload(0)
//     pop(a)
// }
