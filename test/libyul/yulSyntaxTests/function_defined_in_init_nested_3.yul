{
	for {}
		1:bool
		{for {function foo() {}} 1:bool {} {} }
	{}
}
// ----
// SyntaxError 3441: (27-35='function'): Functions cannot be defined inside a for-loop init block.
