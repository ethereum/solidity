{
    let x := 1:invalidType
}
// ====
// dialect: evmTyped
// ----
// TypeError: (15-28): "invalidType" is not a valid type (user defined types are not yet supported).
// TypeError: (10-11): Assigning value of type "invalidType" to variable of type "u256.
