abstract contract a {
    function f() virtual internal;
}
contract b is a {
    function f() internal override
	{
		function() internal x = a.f;
	}
}
// ----
// TypeError: (141-144): Referencing unimplemented function "a.f".
