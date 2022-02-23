object "MyContract" {
  code {
    // this is the constructor.
    // store the creator in the first storage slot
    sstore(0, caller())
    // now return the runtime code using the special functions
    datacopy(0, dataoffset("Runtime"), datasize("Runtime"))
    return(0, datasize("Runtime"))
  }
  object "Runtime" {
    code {
      // runtime - just return the creator
      mstore(0, sload(0))
      return(0, 0x20)
    }
  }
}
