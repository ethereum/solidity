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
	pop(mload(0))
	pop(sload(0))
	pop(msize())
	pop(keccak256(0, 10))
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
// Memory dump:
// Storage dump:
