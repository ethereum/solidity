{
    let x := 1:invalidType
}
// ====
// dialect: evmTyped
// ----
// TypeError 5473: (15-28='1:invalidType'): "invalidType" is not a valid type (user defined types are not yet supported).
// TypeError 3947: (10-11='x'): Assigning value of type "invalidType" to variable of type "u256".
