contract n
{
	function()
	{
		// Used to cause a segfault
		var (x,y) = (1);
		var (z) = ();

		assembly {
			mstore(y, z)
		}
	}
}
// ----
// SyntaxError: (14-129): No visibility specified. Did you intend to add "external"?
// TypeError: (14-129): Fallback function must be defined as "external".
// TypeError: (60-75): Different number of components on the left hand side (2) than on the right hand side (1).
