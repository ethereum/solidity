contract root { function rootFunction() public {} }
contract inter1 is root { function f() public virtual {} }
contract inter2 is root { function f() public virtual {} }
contract derived is root, inter2, inter1 {
	function g() public { f(); rootFunction(); }
	function f() override(inter1, inter2) public {}
}
// ----
