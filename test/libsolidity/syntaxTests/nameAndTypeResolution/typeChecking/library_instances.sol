library X { }

contract Y {
	X abc;
	function foo(X param) private view
	{
		X ofg;
		ofg = abc;
	}
}
// ----
// TypeError: (29-34): The type of a variable cannot be a library.
// TypeError: (50-57): The type of a variable cannot be a library.
// TypeError: (77-82): The type of a variable cannot be a library.
