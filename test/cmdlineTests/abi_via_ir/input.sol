// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

error fileLevelError(uint z);

library L {
	event libraryEvent(uint r);
	error libraryError(uint r);
	error libraryErrorUnused(uint u);
	event libraryEventUnused(uint u);
}

contract C {
	struct S { uint x; }

	event ev(uint y);
	event anon_ev(uint y) anonymous;

	error err(uint z, uint w);

	function f(S memory s) public {
		emit L.libraryEvent(3);
		if (s.x > 1)
			revert fileLevelError(3);
		else
			revert L.libraryError(4);
	}
}
