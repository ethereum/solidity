contract A { function foo() public { new D(); } }
contract C { function foo() public { new A(); } }
contract D is C {}
// ----
// TypeError 7813: (37-42='new D'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (87-92='new A'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
