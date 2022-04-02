object "A" {
  code {
    pop(dataoffset(".other"))
    pop(datasize(".other"))
    pop(datasize("B..other"))
    // "B.other" does not exist.
    pop(datasize("B.other"))
    // ".metadata" is not accessible by definition
    pop(dataoffset(".metadata"))
    pop(datasize(".metadata"))
    pop(dataoffset("B..metadata"))
    pop(datasize("B..metadata"))
  }

  object "B" {
    code {}
    data ".metadata" "Hello, World!"
    data ".other" "Hello, World2!"
  }

  data ".metadata" "Hello, World!"
  data ".other" "Hello, World2!"
}
// ----
// TypeError 3517: (41-49='".other"'): Unknown data object ".other".
// TypeError 3517: (69-77='".other"'): Unknown data object ".other".
// TypeError 3517: (97-107='"B..other"'): Unknown data object "B..other".
// TypeError 3517: (160-169='"B.other"'): Unknown data object "B.other".
// TypeError 3517: (242-253='".metadata"'): Unknown data object ".metadata".
// TypeError 3517: (273-284='".metadata"'): Unknown data object ".metadata".
// TypeError 3517: (306-319='"B..metadata"'): Unknown data object "B..metadata".
// TypeError 3517: (339-352='"B..metadata"'): Unknown data object "B..metadata".
