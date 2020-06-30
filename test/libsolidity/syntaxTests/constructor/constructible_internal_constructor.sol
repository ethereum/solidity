abstract contract C {
	constructor() internal {}
}
contract D is C {
	constructor() { }
}
// ----
// Warning 2462: (23-48): Visibility for constructor is ignored. If you want the contract to be non-deployable, making it "abstract" is sufficient.
