object "A" {
  code {
    datacopy(0, dataoffset("3"), datasize("3"))
  }
  data "1" ""
  data "2" hex"0011"
  data "3" "hello world this is longer than 32 bytes and should still work"
}
// ----
