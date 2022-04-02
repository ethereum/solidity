object "A" {
  code {
    let x := "B"
    pop(datasize(x))
  }

  data "B" hex"00"
}
// ----
// TypeError 9114: (47-55='datasize'): Function expects direct literals as arguments.
