contract A { B x = new B(); }
contract B { C x = new C(); }
contract C { D x = new D(); }
contract D { E x = new E(); }
contract E { F x = new F(); }
contract F { E x = new E(); }
// ----
// TypeError 7813: (19-24): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (49-54): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (79-84): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (109-114): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (139-144): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (169-174): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
