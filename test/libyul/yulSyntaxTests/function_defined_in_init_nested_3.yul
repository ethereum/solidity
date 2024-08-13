{
	for {}
		1
		{for {function foo() {}} 1 {} {} }
	{}
}
// ----
// SyntaxError 3441: (22-30): Functions cannot be defined inside a for-loop init block.
