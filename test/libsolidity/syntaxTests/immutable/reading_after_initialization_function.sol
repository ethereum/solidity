contract C {
	uint immutable x;

	constructor()
	{
		readX();
		x = 3;
		readX();
	}

	function readX() public view returns(uint) {
		return x;
	}
}
// ----
// TypeError 7733: (141-142): Immutable variables cannot be read before they are initialized.
