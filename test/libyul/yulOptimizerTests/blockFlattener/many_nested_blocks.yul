{
  let a := 3
  let b := 4
  {
    a := add(b, 3)
    let c := 5
    {
      b := add(b, 4)
      {
        c := add(a, 5)
	  }
      b := add(a, b)
    }
    a := add(a, c)
  }
}
// ----
// blockFlattener
// {
//     let a := 3
//     let b := 4
//     a := add(b, 3)
//     let c := 5
//     b := add(b, 4)
//     c := add(a, 5)
//     b := add(a, b)
//     a := add(a, c)
// }
