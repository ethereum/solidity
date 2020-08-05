contract n
{
	fallback() external
	{
		// Used to cause a segfault
		(uint x, ) = (1);
		(uint z) = ();

		assembly {
			mstore(x, z)
		}
	}
}
// ----
// TypeError 7364: (69-85): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError 7364: (89-102): Different number of components on the left hand side (1) than on the right hand side (0).
