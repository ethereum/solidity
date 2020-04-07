contract n
{
	fallback() external
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
// TypeError: (69-84): Different number of components on the left hand side (2) than on the right hand side (1).
