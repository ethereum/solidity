{
    // Arguments to ``datasize`` and ``dataoffset`` need to be
    // literals. We cannot simplify their arguments, but we can
    // simplify them as a full expression.
    // ``datacopy`` does not have this restriction.
    let r := "abc"
    let a := datasize("abc")
    let x := dataoffset("abc")
    // should be replaced by a
    let y := datasize("abc")
    datacopy("abc", x, y)
    mstore(a, x)
}
// ----
// commonSubexpressionEliminator
// {
//     let r := "abc"
//     let a := datasize("abc")
//     let x := dataoffset("abc")
//     let y := a
//     datacopy(r, x, a)
//     mstore(a, x)
// }
