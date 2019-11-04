// This used to work pre-0.6.0.
library L {
    // This can be used as an "interface", hence it is allowed.
    function f() public;
}
// ----
// TypeError: (112-132): Library functions must be implemented if declared.
