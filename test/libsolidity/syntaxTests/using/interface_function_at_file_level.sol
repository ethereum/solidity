interface I {
    function g() external pure;
}

using {I.g} for uint;
// ----
// TypeError 4167: (56-59): Only file-level functions and library functions can be attached to a type in a "using" statement
