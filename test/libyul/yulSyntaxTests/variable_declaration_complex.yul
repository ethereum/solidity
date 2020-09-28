{
    function add(a:u256, b:u256) -> c:u256 {}
    let y:u256 := 2:u256
    let x:u256 := add(7:u256, add(6:u256, y))
}
// ====
// dialect: yul