{
    function f(a, b, c) -> r, t {
        r := lt(a, b)
        t := not(c)
    }
    let x, y := f(1, 2, true, false)
}
// ----
// TypeError 7000: (100-101): Function "f" expects 3 arguments but got 4.
