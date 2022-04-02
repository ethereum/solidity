// This used to work pre-0.6.0.
library L {
    // This can be used as an "interface", hence it is allowed.
    function f() public;
}
// ----
// TypeError 9231: (112-132='function f() public;'): Library functions must be implemented if declared.
