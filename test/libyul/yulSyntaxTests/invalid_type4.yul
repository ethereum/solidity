{
    switch 1
    case 8: invalidType {}
}
// ====
// dialect: evmTyped
// ----
// TypeError 3781: (24-38): Expected a value of type "u256" but got "invalidType"
// TypeError 5473: (24-38): "invalidType" is not a valid type (user defined types are not yet supported).
