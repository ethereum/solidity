error E();

contract C {
    function() internal pure x = E;
}
// ----
// TypeError 7407: (58-59): Type error E() is not implicitly convertible to expected type function () pure. Special functions can not be converted to function types.
