struct this { uint a; }
struct super { uint b; }
struct _ { uint c; }

contract C {
	this a;
	super b;
	_ c;
}
// ----
// DeclarationError 3726: (0-23): The name "this" is reserved.
// DeclarationError 3726: (24-48): The name "super" is reserved.
// DeclarationError 3726: (49-69): The name "_" is reserved.
// Warning 2319: (0-23): This declaration shadows a builtin symbol.
// Warning 2319: (24-48): This declaration shadows a builtin symbol.
