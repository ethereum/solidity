{
    let a := mload(0)
    let b := mload(1)
    for {
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
// ====
// step: ssaAndBack
// ----
// {
//     let a := mload(0)
//     let b := mload(1)
//     for { } lt(mload(a), mload(b)) { a := mload(b) }
//     {
//         let b_4 := mload(a)
//         a := mload(b_4)
//         b := mload(a)
//     }
// }
