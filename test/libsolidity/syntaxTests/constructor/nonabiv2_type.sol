contract C {
	constructor(uint[][][] memory t) {}
}
// ----
// TypeError 4957: (26-45): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature. Alternatively, make the contract abstract and supply the constructor arguments from a derived contract.
