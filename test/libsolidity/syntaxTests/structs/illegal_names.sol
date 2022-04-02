struct this { uint a; }
struct super { uint b; }
struct _ { uint c; }

contract C {
	this a;
	super b;
	_ c;
}
// ----
// DeclarationError 3726: (0-23='struct this { uint a; }'): The name "this" is reserved.
// DeclarationError 3726: (24-48='struct super { uint b; }'): The name "super" is reserved.
// DeclarationError 3726: (49-69='struct _ { uint c; }'): The name "_" is reserved.
// Warning 2319: (0-23='struct this { uint a; }'): This declaration shadows a builtin symbol.
// Warning 2319: (24-48='struct super { uint b; }'): This declaration shadows a builtin symbol.
