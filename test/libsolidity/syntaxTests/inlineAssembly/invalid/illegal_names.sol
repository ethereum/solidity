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
// DeclarationError 4113: (105-136='function this() {             }'): The identifier name "this" is reserved.
// DeclarationError 4113: (149-181='function super() {             }'): The identifier name "super" is reserved.
// DeclarationError 4113: (194-222='function _() {             }'): The identifier name "_" is reserved.
// DeclarationError 4113: (323-327='this'): The identifier name "this" is reserved.
// DeclarationError 4113: (368-373='super'): The identifier name "super" is reserved.
// DeclarationError 4113: (414-415='_'): The identifier name "_" is reserved.
// DeclarationError 4113: (546-550='this'): The identifier name "this" is reserved.
// DeclarationError 4113: (595-600='super'): The identifier name "super" is reserved.
// DeclarationError 4113: (645-646='_'): The identifier name "_" is reserved.
// DeclarationError 4113: (759-763='this'): The identifier name "this" is reserved.
// DeclarationError 3859: (759-763='this'): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 4113: (785-790='super'): The identifier name "super" is reserved.
// DeclarationError 3859: (785-790='super'): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 4113: (812-813='_'): The identifier name "_" is reserved.
