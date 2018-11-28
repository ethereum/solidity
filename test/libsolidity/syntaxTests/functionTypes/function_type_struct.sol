library L
{
	struct Nested
	{
		uint y;
	}
	function f(function(Nested memory) external) external pure {}
}
