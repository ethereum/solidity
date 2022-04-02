contract C {
    uint immutable super;
    uint immutable _;
    uint immutable this;
}
// ----
// DeclarationError 3726: (17-37='uint immutable super'): The name "super" is reserved.
// DeclarationError 3726: (43-59='uint immutable _'): The name "_" is reserved.
// DeclarationError 3726: (65-84='uint immutable this'): The name "this" is reserved.
// Warning 2319: (17-37='uint immutable super'): This declaration shadows a builtin symbol.
// Warning 2319: (65-84='uint immutable this'): This declaration shadows a builtin symbol.
