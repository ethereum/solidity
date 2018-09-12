pragma solidity >0.4.24;

contract C
{
	function f(uint x) public pure {
		do
			uint y;
		while (x > 0);
	}
}
// ----
// SyntaxError: (81-87): Variable declarations can only be used inside blocks.
