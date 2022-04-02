uint constant this = 1;
uint constant super = 2;
uint constant _ = 3;
contract C {
	address this;
	int super;
	mapping (address => address) _;
}

contract D {
	address[] this;
	struct _ { uint super; }
}
// ----
// DeclarationError 3726: (0-22='uint constant this = 1'): The name "this" is reserved.
// DeclarationError 3726: (24-47='uint constant super = 2'): The name "super" is reserved.
// DeclarationError 3726: (49-68='uint constant _ = 3'): The name "_" is reserved.
// DeclarationError 3726: (84-96='address this'): The name "this" is reserved.
// DeclarationError 3726: (99-108='int super'): The name "super" is reserved.
// DeclarationError 3726: (111-141='mapping (address => address) _'): The name "_" is reserved.
// DeclarationError 3726: (160-174='address[] this'): The name "this" is reserved.
// DeclarationError 3726: (177-201='struct _ { uint super; }'): The name "_" is reserved.
// DeclarationError 3726: (188-198='uint super'): The name "super" is reserved.
// Warning 2519: (84-96='address this'): This declaration shadows an existing declaration.
// Warning 2519: (99-108='int super'): This declaration shadows an existing declaration.
// Warning 2519: (111-141='mapping (address => address) _'): This declaration shadows an existing declaration.
// Warning 2519: (160-174='address[] this'): This declaration shadows an existing declaration.
// Warning 2519: (177-201='struct _ { uint super; }'): This declaration shadows an existing declaration.
// Warning 2319: (0-22='uint constant this = 1'): This declaration shadows a builtin symbol.
// Warning 2319: (24-47='uint constant super = 2'): This declaration shadows a builtin symbol.
