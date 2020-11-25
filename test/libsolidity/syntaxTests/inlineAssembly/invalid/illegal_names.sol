contract C {
    function f() public {
        // reserved function names
        assembly {
            function this() {
            }
            function super() {
            }
            function _() {
            }
        }

        // reserved names as function argument
        assembly {
            function a(this) {
            }
            function b(super) {
            }
            function c(_) {
            }
        }

        // reserved names as function return parameter
        assembly {
            function d() -> this {
            }
            function g() -> super {
            }
            function c() -> _ {
            }
        }

        // reserved names as variable declaration
        assembly {
            let this := 1
            let super := 1
            let _ := 1
        }
    }
}
// ----
// DeclarationError 4113: (105-136): The identifier name "this" is reserved.
// DeclarationError 4113: (149-181): The identifier name "super" is reserved.
// DeclarationError 4113: (194-222): The identifier name "_" is reserved.
// DeclarationError 4113: (323-327): The identifier name "this" is reserved.
// DeclarationError 4113: (368-373): The identifier name "super" is reserved.
// DeclarationError 4113: (414-415): The identifier name "_" is reserved.
// DeclarationError 4113: (546-550): The identifier name "this" is reserved.
// DeclarationError 4113: (595-600): The identifier name "super" is reserved.
// DeclarationError 4113: (645-646): The identifier name "_" is reserved.
// DeclarationError 4113: (759-763): The identifier name "this" is reserved.
// DeclarationError 3859: (759-763): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 4113: (785-790): The identifier name "super" is reserved.
// DeclarationError 3859: (785-790): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 4113: (812-813): The identifier name "_" is reserved.
