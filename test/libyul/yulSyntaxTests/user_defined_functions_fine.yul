{
    function f(a:u256, b:u256, c:bool) -> r:bool, t {
        r := lt(a, b)
        t := bool_to_u256(not(c))
    }
    let x: bool, y: u256 := f(1, 2: u256, true)
}
// ====
// dialect: evmTyped
// ----
