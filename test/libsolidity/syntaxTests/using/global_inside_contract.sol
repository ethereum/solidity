contract C {
    using {f} for uint global;
}
function f(uint) pure{}
// ----
// SyntaxError 3367: (17-43): "global" can only be used at file level.
// TypeError 8841: (17-43): Can only use "global" with user-defined types.
