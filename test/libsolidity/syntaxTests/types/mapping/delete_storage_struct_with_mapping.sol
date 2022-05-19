contract D {
	struct Test {
		mapping(address => uint) balances;
	}

	Test test;

	constructor()
	{
		delete test;
	}
}
// ----
// TypeError 9767: (102-113): Unary operator delete cannot be applied to type struct D.Test storage ref. Contains a (possibly nested) mapping
