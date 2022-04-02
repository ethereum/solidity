contract C {
	constructor() { new C(); }
}
// ----
// TypeError 7813: (30-35='new C'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
