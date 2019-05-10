{
  {
    function f() -> x { x := 1 }
    { mstore(f(), 2) }
    { mstore(f(), 2) }
  }
}
// ====
// step: blockOutliner
// ----
// {
//     {
//         { outlined$43$() }
//         { outlined$43$() }
//     }
//     function f() -> x
//     { x := 1 }
//     function outlined$43$()
//     { mstore(f(), 2) }
// }
