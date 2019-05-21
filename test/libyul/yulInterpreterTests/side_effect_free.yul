{
	// These can be removed by the optimizer and should not
	// appear in the trace.
	pop(gas())
	pop(extcodesize(0))
	pop(extcodehash(0))
	pop(returndatasize())
	pop(sload(0))
	pop(pc())
	pop(msize())
}
// ----
// Trace:
// Memory dump:
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000000: 0000000000000000000000000000000000000000000000000000000000000000
