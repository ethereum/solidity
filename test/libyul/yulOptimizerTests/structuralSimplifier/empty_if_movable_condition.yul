{ let a := mload(0) if a {} }
// ====
// step: structuralSimplifier
// ----
// {
//     let a := mload(0)
//     pop(a)
// }
