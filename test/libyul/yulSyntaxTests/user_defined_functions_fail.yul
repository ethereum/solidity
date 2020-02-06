{
    function f(a:u256, b:u256, c:bool) -> r:bool, t {
        r := lt(a, b)
        t := bool_to_u256(not(c))
    }
    let x, y: bool := f(1, 2: u256, true)
}
// ====
// dialect: evmTyped
// ----
// TypeError: (126-127): Assigning value of type "bool" to variable of type "u256.
// TypeError: (129-136): Assigning value of type "u256" to variable of type "bool.
