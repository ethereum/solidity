{
    function f(a: invalidType) -> b: invalidType {}
}
// ====
// dialect: evmTyped
// ----
// TypeError: (17-31): "invalidType" is not a valid type (user defined types are not yet supported).
// TypeError: (36-50): "invalidType" is not a valid type (user defined types are not yet supported).
