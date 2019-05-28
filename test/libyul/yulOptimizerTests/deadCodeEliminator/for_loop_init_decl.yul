{
  for { stop() let i_0 := 0 } lt(i_0,2) { i_0 := add(i_0,1) }
  {
    let i_1 := i_0
  }
}
// ====
// step: deadCodeEliminator
// ----
// { stop() }
