pragma solidity >0.4.24;

contract C
{
	function f(uint x) public pure {
		while (x > 0)
			uint y;
	}
}
// ----
// SyntaxError: (92-98): Variable declarations can only be used inside blocks.
