{
    {
    	function z() -> x { x := y() }
    	function y() -> x { x := z() }
    }
    {
    	function z() -> x { x := y() }
    	function y() -> x { x := z() }
    }
}
// ----
// step: circularReferencesPruner
//
// { { } }
