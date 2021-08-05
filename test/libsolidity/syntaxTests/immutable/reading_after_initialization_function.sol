contract C {
    uint immutable x ;

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
// TypeError 7733: (145-146): Immutable variables cannot be read before they are initialized.
