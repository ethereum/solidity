// Exception for the illegal name list. External interface events
contract C {
	event this();
	event super();
	event _();
}
// ----
// Warning 2319: (80-93='event this();'): This declaration shadows a builtin symbol.
// Warning 2319: (95-109='event super();'): This declaration shadows a builtin symbol.
