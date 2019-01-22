{
    for {
      let a := mload(0)
      let b := mload(1)
    }
    lt(mload(a),mload(b))
    {
       a := mload(b)
    }
    {
        b := mload(a)
        a := mload(b)
        a := mload(b)
        a := mload(b)
        b := mload(a)
    }
}
// ----
// ssaAndBack
// {
//     for {
//         let a := mload(0)
//         let b := mload(1)
//     }
//     lt(mload(a), mload(b))
//     {
//         a := mload(b)
//     }
//     {
//         let b_3 := mload(a)
//         pop(mload(b_3))
//         pop(mload(b_3))
//         let a_6 := mload(b_3)
//         b := mload(a_6)
//     }
// }
