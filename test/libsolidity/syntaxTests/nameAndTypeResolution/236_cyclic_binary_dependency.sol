contract A { function f() public { new B(); } }
contract B { function f() public { new C(); } }
contract C { function f() public { new A(); } }
// ----
// TypeError 7813: (35-40='new B'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (83-88='new C'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (131-136='new A'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
