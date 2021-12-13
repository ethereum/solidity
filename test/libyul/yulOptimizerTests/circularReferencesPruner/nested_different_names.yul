{
    {
    	function a() -> x { x := b() }
    	function b() -> y { y := a() }
    }
    {
    	function c() -> z { z := d() }
    	function d() -> w { w := c() }
    }
}
// ----
// step: circularReferencesPruner
//
// { { } }
