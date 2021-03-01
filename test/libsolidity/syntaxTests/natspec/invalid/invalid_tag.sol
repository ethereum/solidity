/// @a&b test
contract C {
	/// @custom:x^y test2
	function f() public pure {}
	/// @custom:
	function g() public pure {}
	/// @custom:abcDEF
	function h() public pure {}
	/// @custom:abc-def
	function i() public pure {}
}
// ----
// DocstringParsingError 6546: (0-14): Documentation tag @a&b not valid for contracts.
// DocstringParsingError 2968: (28-49): Invalid character in custom tag @custom:x^y. Only lowercase letters and "-" are permitted.
// DocstringParsingError 6546: (80-92): Documentation tag @custom: not valid for functions.
// DocstringParsingError 2968: (123-141): Invalid character in custom tag @custom:abcDEF. Only lowercase letters and "-" are permitted.
