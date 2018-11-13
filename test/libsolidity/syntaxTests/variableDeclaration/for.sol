pragma solidity >0.4.24;

contract C
{
	function f(uint x) public pure {
		for (uint i = 0; i < x; ++i)
			uint y;
	}
}
// ----
// SyntaxError: (107-113): Variable declarations can only be used inside blocks.
