contract C {
	modifier this { _; }
	modifier super { _; }
	modifier _ { _; }
}
// ----
// DeclarationError 3726: (14-34='modifier this { _; }'): The name "this" is reserved.
// DeclarationError 3726: (36-57='modifier super { _; }'): The name "super" is reserved.
// DeclarationError 3726: (59-76='modifier _ { _; }'): The name "_" is reserved.
// Warning 2319: (14-34='modifier this { _; }'): This declaration shadows a builtin symbol.
// Warning 2319: (36-57='modifier super { _; }'): This declaration shadows a builtin symbol.
