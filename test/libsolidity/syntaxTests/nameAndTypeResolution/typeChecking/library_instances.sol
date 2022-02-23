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
// TypeError 1130: (29-30): Invalid use of a library name.
// TypeError 1130: (50-51): Invalid use of a library name.
// TypeError 1130: (77-78): Invalid use of a library name.
