contract C {
    function _() internal returns(uint) {
        return 1;
    }

    function super() internal {
    }

    function this() internal {
    }
}
// ----
// DeclarationError 3726: (17-78): The name "_" is reserved.
// DeclarationError 3726: (84-117): The name "super" is reserved.
// DeclarationError 3726: (123-155): The name "this" is reserved.
// Warning 2319: (84-117): This declaration shadows a builtin symbol.
// Warning 2319: (123-155): This declaration shadows a builtin symbol.
