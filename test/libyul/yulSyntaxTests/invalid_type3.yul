{
    function f(a: invalidType) -> b: invalidType {}
}
// ====
// dialect: evmTyped
// ----
// TypeError 5473: (17-31): "invalidType" is not a valid type (user defined types are not yet supported).
// TypeError 5473: (36-50): "invalidType" is not a valid type (user defined types are not yet supported).
