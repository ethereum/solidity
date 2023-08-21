contract C {
	///@return
	modifier m22 virtual { _; }
}

contract D is C {
	modifier m22 override { _; }
}
// ----
// DocstringParsingError 6546: (14-24): Documentation tag @return not valid for modifiers.
