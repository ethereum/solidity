// Exception for the illegal name list. External interface events
contract C {
	event this();
	event super();
	event _();
}
// ----
// Warning 2319: (80-93): This declaration shadows a builtin symbol.
// Warning 2319: (95-109): This declaration shadows a builtin symbol.
