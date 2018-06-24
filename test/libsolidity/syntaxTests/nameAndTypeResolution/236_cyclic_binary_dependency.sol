contract A { function f() public { new B(); } }
contract B { function f() public { new C(); } }
contract C { function f() public { new A(); } }
// ----
// TypeError: (131-136): Circular reference for contract creation (cannot create instance of derived or same contract).
