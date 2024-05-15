object "test"
{
  code
  {
    f()
    g()
    h()
    i()
    function f()
    {
      let x := datasize("A")
    }
    function g()
    {
      let x := datasize(hex"41")
    }
    function h()
    {
      let x := datasize("\x41")
    }
    function i()
    {
      let x := datasize("\u0041")
    }
  }
  data 'A' "A"
}

// ----
// step: equivalentFunctionCombiner
//
// object "test" {
//     code {
//         f()
//         f()
//         f()
//         f()
//         function f()
//         { let x := datasize("A") }
//         function g()
//         { let x_1 := datasize("A") }
//         function h()
//         { let x_2 := datasize("A") }
//         function i()
//         { let x_3 := datasize("A") }
//     }
//     data "A" hex"41"
// }
