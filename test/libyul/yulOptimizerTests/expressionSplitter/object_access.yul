object "main" {
    code {
        // We should never split arguments to ``dataoffset``
        // or ``datasize`` because they need to be literals
        let x := dataoffset("abc")
        let y := datasize("abc")
        // datacopy is fine, though
        datacopy(mload(0), mload(1), mload(2))
    }
    data "abc" "Hello, World!"
}
// ====
// step: expressionSplitter
// ----
// {
//     let x := dataoffset("abc")
//     let y := datasize("abc")
//     let _1 := 2
//     let _2 := mload(_1)
//     let _3 := 1
//     let _4 := mload(_3)
//     let _5 := 0
//     let _6 := mload(_5)
//     datacopy(_6, _4, _2)
// }
