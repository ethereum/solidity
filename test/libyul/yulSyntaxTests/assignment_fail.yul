{
    let x:u256
    let y := x
    let z:bool
    z := y
    y := z
}
// ====
// dialect: evmTyped
// ----
// TypeError: (51-52): Assigning a value of type "u256" to a variable of type "bool".
// TypeError: (62-63): Assigning a value of type "bool" to a variable of type "u256".
