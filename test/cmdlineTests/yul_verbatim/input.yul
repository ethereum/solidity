{
    let x := 2
    let y := sub(x, 2)
    let t := verbatim_2i_1o("abc", x, y)
    sstore(t, x)
    let r := verbatim_0i_1o("def")
    verbatim_0i_0o("xyz")
    // more than 32 bytes
    verbatim_0i_0o(hex"01020304050607090001020304050607090001020304050607090001020102030405060709000102030405060709000102030405060709000102")
    r := 9
}
