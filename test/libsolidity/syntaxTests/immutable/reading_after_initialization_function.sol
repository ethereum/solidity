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
