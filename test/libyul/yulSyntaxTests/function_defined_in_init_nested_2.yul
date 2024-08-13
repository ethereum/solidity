{
	for { for {function foo() {}} 1 {} {} } 1 {} {}
}
// ----
// SyntaxError 3441: (14-22): Functions cannot be defined inside a for-loop init block.
