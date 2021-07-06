object "A" {
  code {
    sstore(dataoffset("0"), dataoffset("1"))
    sstore(0, datasize(".mightbereservedinthefuture"))
  }

  data "0"  "UVW"
  data ".metadata" "ABC"
  data "1" "XYZ"
  data ".mightbereservedinthefuture" "TRS"
}
// ----
// TypeError 3517: (90-119): Unknown data object ".mightbereservedinthefuture".
