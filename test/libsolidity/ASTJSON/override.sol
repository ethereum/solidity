contract A {
	function faa() public virtual {}
}
abstract contract B is A {
	function foo() public virtual;
	function faa() public virtual override {}
}
contract C is B {
	function foo() public override { }
	function faa() public override { }
}
// ----
