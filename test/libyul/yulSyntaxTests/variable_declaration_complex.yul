{
    function add_fn(a:u256, b:u256) -> c:u256 {}
    let y:u256 := 2:u256
    let x:u256 := add_fn(7:u256, add_fn(6:u256, y))
}
// ====
// dialect: evmTyped
// ----
