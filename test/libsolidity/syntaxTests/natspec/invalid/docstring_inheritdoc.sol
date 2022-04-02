contract C {
    /// @inheritdoc X
    function f() internal {
    }
}
// ----
// DocstringParsingError 9397: (17-34='/// @inheritdoc X'): Documentation tag @inheritdoc references inexistent contract "X".
