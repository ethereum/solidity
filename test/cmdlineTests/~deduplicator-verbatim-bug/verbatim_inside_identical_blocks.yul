{
  let special := 0xFFFFFFFFFFFF
  let input := sload(0)
  let output

  switch input
  case 0x00 {
      output := verbatim_1i_1o(hex"506000", special)
  }
  case 0x01 {
      output := 1
  }
  case 0x02 {
      output := verbatim_1i_1o(hex"506002", special)
  }
  case 0x03 {
      output := 3
  }

  sstore(0, output)
}
