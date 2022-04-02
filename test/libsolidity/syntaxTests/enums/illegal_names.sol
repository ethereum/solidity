enum this {
    a
}
enum super {
    b
}
enum _ {
    c
}

enum E {
    this,
    super,
    _
}

contract C {
    this a;
    super b;
    _ c;
    E e;
}
// ----
// DeclarationError 3726: (0-19='enum this {     a }'): The name "this" is reserved.
// DeclarationError 3726: (20-40='enum super {     b }'): The name "super" is reserved.
// DeclarationError 3726: (41-57='enum _ {     c }'): The name "_" is reserved.
// DeclarationError 3726: (72-76='this'): The name "this" is reserved.
// DeclarationError 3726: (82-87='super'): The name "super" is reserved.
// DeclarationError 3726: (93-94='_'): The name "_" is reserved.
// Warning 2319: (0-19='enum this {     a }'): This declaration shadows a builtin symbol.
// Warning 2319: (20-40='enum super {     b }'): This declaration shadows a builtin symbol.
