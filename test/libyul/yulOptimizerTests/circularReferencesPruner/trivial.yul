{
    function f() -> x { x := g() }
    function g() -> x { x := f() }
}
// ----
// step: circularReferencesPruner
//
// { { } }
