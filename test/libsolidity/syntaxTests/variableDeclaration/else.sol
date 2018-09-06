pragma solidity >0.4.24;

contract C
{
	function f(uint x) public pure {
		if (x > 0)
			{uint y;}
		else
			uint z;
	}
}
// ----
// SyntaxError: (109-115): Variable declarations can only be used inside blocks.
