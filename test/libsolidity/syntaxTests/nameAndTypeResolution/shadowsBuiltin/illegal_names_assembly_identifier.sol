contract C {
    function f() public {
        assembly {
            let super := 1
            let this := 1
            let _ := 1
        }
    }
}
// ----
// DeclarationError 4113: (74-79='super'): The identifier name "super" is reserved.
// DeclarationError 3859: (74-79='super'): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 4113: (101-105='this'): The identifier name "this" is reserved.
// DeclarationError 3859: (101-105='this'): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 4113: (127-128='_'): The identifier name "_" is reserved.
