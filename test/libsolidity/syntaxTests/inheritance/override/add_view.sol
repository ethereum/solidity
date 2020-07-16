contract B { function f() virtual public {} }
contract C is B { function f() override public view {} }
// ----
