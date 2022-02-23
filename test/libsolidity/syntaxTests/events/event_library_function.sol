library L {
	function f() public {
		int x = 1;
	}
}

contract C {
	event Test(function() external indexed);

	function g() public {
		Test(L.f);
	}
}

contract D {
	event Test(function() external);

	function f() public {
		Test(L.f);
	}
}

contract E {
	event Test(function() external indexed);

	using L for D;

	function k() public {
		Test(D.f);
	}
}
// ----
// TypeError 9553: (140-143): Invalid type for argument in function call. Invalid implicit conversion from function () to function () external requested. Special functions can not be converted to function types.
// TypeError 9553: (230-233): Invalid type for argument in function call. Invalid implicit conversion from function () to function () external requested. Special functions can not be converted to function types.
// TypeError 9553: (345-348): Invalid type for argument in function call. Invalid implicit conversion from function D.f() to function () external requested. Special functions can not be converted to function types.
