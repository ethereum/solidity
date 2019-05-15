{
  let x := 0xff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00
  let y := 0x1100ff00ff00ff001100ff00ff001100ff00ff001100ff00ff001100ff001100
  let z := 0xffff0000ffff0000ffff0000ffff0000ff00ff00ffff0000ffff0000ffff0000
  let w := 0xffffffff000000ffffef000001feff000067ffefff0000ff230002ffee00fff7
}
// ====
// step: constantOptimiser
// ----
// {
//     let x := 0xff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00
//     let y := 0x1100ff00ff00ff001100ff00ff001100ff00ff001100ff00ff001100ff001100
//     let z := 0xffff0000ffff0000ffff0000ffff0000ff00ff00ffff0000ffff0000ffff0000
//     let w := 0xffffffff000000ffffef000001feff000067ffefff0000ff230002ffee00fff7
// }
