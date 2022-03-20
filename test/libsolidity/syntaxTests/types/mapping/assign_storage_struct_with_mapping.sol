contract D {
	struct Test {
		mapping(address => uint) balances;
	}

	Test test;

	constructor()
	{
		test = Test();
	}
}
// ----
// TypeError 9214: (102-106): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9515: (109-115): Struct containing a (nested) mapping cannot be constructed.
