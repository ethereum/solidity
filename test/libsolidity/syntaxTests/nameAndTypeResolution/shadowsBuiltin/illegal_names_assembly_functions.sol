contract C {
    function f() public pure {
        assembly {
            function this() {
            }
            function super() {
            }
            function _() {
            }
        }
    }
}
// ----
// DeclarationError 4113: (75-106='function this() {             }'): The identifier name "this" is reserved.
// DeclarationError 4113: (119-151='function super() {             }'): The identifier name "super" is reserved.
// DeclarationError 4113: (164-192='function _() {             }'): The identifier name "_" is reserved.
