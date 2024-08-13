{
    function f(a, b, c) -> r, t {
        r := lt(a, b)
        t := not(c)
    }
    let x, y := f(1, 2, true)
}
// ----
