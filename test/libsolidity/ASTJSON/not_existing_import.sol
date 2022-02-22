import "notexisting.sol" as NotExisting;
contract C is NotExisting.X
{
	NotExisting.SomeStruct public myStruct;
	constructor() {}
}

// ----
// failAfter: Parsed
