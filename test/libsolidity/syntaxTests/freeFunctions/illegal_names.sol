function this() {}
function super() {}
function _() {}

contract C {
	function test() public {
		this();
		super();
		_();
	}
}
// ----
// DeclarationError 3726: (0-18='function this() {}'): The name "this" is reserved.
// DeclarationError 3726: (19-38='function super() {}'): The name "super" is reserved.
// DeclarationError 3726: (39-54='function _() {}'): The name "_" is reserved.
// Warning 2319: (0-18='function this() {}'): This declaration shadows a builtin symbol.
// Warning 2319: (19-38='function super() {}'): This declaration shadows a builtin symbol.
