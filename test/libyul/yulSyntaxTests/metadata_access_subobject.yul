object "A" {
	code {
		sstore(dataoffset(".metadata.x"), 0)
	}

	object ".metadata" {
		code {}
		data "x" "ABC"
	}
}
// ----
// TypeError 3517: (41-54='".metadata.x"'): Unknown data object ".metadata.x".
