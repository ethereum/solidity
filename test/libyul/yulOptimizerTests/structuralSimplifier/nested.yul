{ if 1 { if 1 { for { mstore(0, 0) } 0 {} { mstore(2, 3) }  if 0 { mstore(1, 2) } } } }
// ====
// step: structuralSimplifier
// ----
// {
//     mstore(0, 0)
// }
