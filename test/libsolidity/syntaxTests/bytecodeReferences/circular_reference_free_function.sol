function f()
{
	new D();
}

contract D
{
	receive() external payable { f; }
}
// ----
// TypeError 7813: (16-21): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
