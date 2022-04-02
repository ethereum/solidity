contract C {
    function f(uint super) public {
    }
    function g(uint this) public {
    }
    function h(uint _) public {
    }
    function i() public returns (uint super) {
        return 1;
    }
    function j() public returns (uint this) {
        return 1;
    }
    function k() public returns (uint _) {
        return 1;
    }
}
// ----
// DeclarationError 3726: (28-38='uint super'): The name "super" is reserved.
// DeclarationError 3726: (70-79='uint this'): The name "this" is reserved.
// DeclarationError 3726: (111-117='uint _'): The name "_" is reserved.
// DeclarationError 3726: (167-177='uint super'): The name "super" is reserved.
// DeclarationError 3726: (238-247='uint this'): The name "this" is reserved.
// DeclarationError 3726: (308-314='uint _'): The name "_" is reserved.
// Warning 2319: (28-38='uint super'): This declaration shadows a builtin symbol.
// Warning 2319: (70-79='uint this'): This declaration shadows a builtin symbol.
// Warning 2319: (167-177='uint super'): This declaration shadows a builtin symbol.
// Warning 2319: (238-247='uint this'): This declaration shadows a builtin symbol.
