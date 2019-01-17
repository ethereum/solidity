library L
{
	struct Nested
	{
		Non y;
	}
	function f(function(Nested memory) external) external pure {}
}
// ----
// DeclarationError: (32-35): Identifier not found or not unique.
// TypeError: (63-76): Internal type cannot be used for external function type.
