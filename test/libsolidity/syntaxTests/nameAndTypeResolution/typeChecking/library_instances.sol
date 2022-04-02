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
// TypeError 1130: (29-30='X'): Invalid use of a library name.
// TypeError 1130: (50-51='X'): Invalid use of a library name.
// TypeError 1130: (77-78='X'): Invalid use of a library name.
