---
layout: docs
title: Arrays and Structs
permalink: /docs/arrays-and-structs/
---

Complex types, i.e. types which do not always fit into 256 bits have to be handled
more carefully than the value-types we have already seen. Since copying
them can be quite expensive, we have to think about whether we want them to be
stored in **memory** (which is not persisting) or **storage** (where the state
variables are held).

### Data location

Every complex type, i.e. *arrays* and *structs*, has an additional
annotation, the "data location", about whether it is stored in memory or in storage. Depending on the
context, there is always a default, but it can be overridden by appending
either `storage` or `memory` to the type. The default for function parameters (including return parameters) is `memory`, the default for local variables is `storage` and the location is forced
to `storage` for state variables (obviously).

There is also a third data location, "calldata" which is a non-modifyable
non-persistent area whether function arguments are stored. Function parameters
(not return parameters) of external functions are forced to "calldata" and
it behaves mostly like memory.

Data locations are important because they change how assignments behave:
Assignments between storage and memory and also to a state variable (even from other state variables)
always create an independent copy.
Assignments to local storage variables only assign a reference though, and
this reference always points to the state variable even if the latter is changed
in the meantime.
On the other hand, assignments from a memory stored reference type to another
memory-stored reference type does not create a copy.

{% highlight javascript %}
contract c {
  uint[] x; // the data location of x is storage
  // the data location of memoryArray is memory
  function f(uint[] memoryArray) {
    x = memoryArray; // works, copies the whole array to storage
    var y = x; // works, assigns a pointer, data location of y is storage
    y[7]; // fine, returns the 8th element
    y.length = 2; // fine, modifies x through y
    delete x; // fine, clears the array, also modifies y
    // The following does not work; it would need to create a new temporary /
    // unnamed array in storage, but storage is "statically" allocated:
    // y = memoryArray;
    // This does not work either, since it would "reset" the pointer, but there
    // is no sensible location it could point to.
    // delete y;
	g(x); // calls g, handing over a reference to x
	h(x); // calls h and creates an independent, temporary copy in memory
  }
  function g(uint[] storage storageArray) internal {}
  function h(uint[] memoryArray) {}
}
{% endhighlight %}

**Summary:**

Forced data location:
 - parameters (not return) of external functions: calldata
 - state variables: storage

Default data location:
 - parameters (also return) of functions: memory
 - all other local variables: storage

### Arrays

Arrays can have a compile-time fixed size or they can be dynamic.
For storage arrays, the element type can be arbitrary (i.e. also other
arrays, mappings or structs). For memory arrays, it cannot be a mapping and
has to be an ABI type if it is an argument of a publicly-visible function.

An array of fixed size `k` and element type `T` is written as `T[k]`,
an array of dynamic size as `T[]`. As an example, an array of 5 dynamic
arrays of `uint` is `uint[][5]` (note that the notation is reversed when
compared to some other languages). To access the second uint in the
third dynamic array, you use `x[2][1]` (indices are zero-based and
access works in the opposite way of the declaration, i.e. `x[2]`
shaves off one level in the type from the right).

Variables of type `bytes` and `string` are special arrays. A `bytes` is similar to `byte[]`,
but it is packed tightly in calldata. `string` is equal to `bytes` but does not allow
length or index access (for now).

*length*: Arrays have a `length` member to hold their number of elements.
Dynamic arrays can be resized in storage (not in memory) by changing the
`.length` member. This does not happen automatically when attempting to access elements outside the current length. The size of memory arrays is fixed (but dynamic, i.e. it can depend on runtime parameters) once they are created.
*push*: Dynamic storage arrays and `bytes` (not `string`) have a member function called `push` that can be used to append an element at the end of the array. The function returns the new length.


{% highlight javascript %}
contract ArrayContract {
  uint[2**20] m_aLotOfIntegers;
  // Note that the following is not a pair of arrays but an array of pairs.
  bool[2][] m_pairsOfFlags;
  // newPairs is stored in memory - the default for function arguments
  function setAllFlagPairs(bool[2][] newPairs) {
    // assignment to a storage array replaces the complete array
    m_pairsOfFlags = newPairs;
  }
  function setFlagPair(uint index, bool flagA, bool flagB) {
    // access to a non-existing index will throw an exception
    m_pairsOfFlags[index][0] = flagA;
    m_pairsOfFlags[index][1] = flagB;
  }
  function changeFlagArraySize(uint newSize) {
    // if the new size is smaller, removed array elements will be cleared
    m_pairsOfFlags.length = newSize;
  }
  function clear() {
    // these clear the arrays completely
    delete m_pairsOfFlags;
    delete m_aLotOfIntegers;
    // identical effect here
    m_pairsOfFlags.length = 0;
  }
  bytes m_byteData;
  function byteArrays(bytes data) {
    // byte arrays ("bytes") are different as they are stored without padding,
    // but can be treated identical to "uint8[]"
    m_byteData = data;
    m_byteData.length += 7;
    m_byteData[3] = 8;
    delete m_byteData[2];
  }
  function addFlag(bool[2] flag) returns (uint) {
    return m_pairsOfFlags.push(flag);
  }
}
{% endhighlight %}

### Structs

Solidity provides a way to define new types in the form of structs, which is
shown in the following example:

{% highlight javascript %}
contract CrowdFunding {
  // Defines a new type with two fields.
  struct Funder {
    address addr;
    uint amount;
  }
  struct Campaign {
    address beneficiary;
    uint fundingGoal;
    uint numFunders;
    uint amount;
    mapping (uint => Funder) funders;
  }
  uint numCampaigns;
  mapping (uint => Campaign) campaigns;
  function newCampaign(address beneficiary, uint goal) returns (uint campaignID) {
    campaignID = numCampaigns++; // campaignID is return variable
    // Creates new struct and saves in storage. We leave out the mapping type.
    campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0);
  }
  function contribute(uint campaignID) {
    Campaign c = campaigns[campaignID];
	// Creates a new temporary memory struct, initialised with the given values
	// and copies it over to storage.
	// Note that you can also use Funder(msg.sender, msg.value) to initialise.
    c.funders[c.numFunders++] = Funder({addr: msg.sender, amount: msg.value});
    c.amount += msg.value;
  }
  function checkGoalReached(uint campaignID) returns (bool reached) {
    Campaign c = campaigns[campaignID];
    if (c.amount < c.fundingGoal)
      return false;
    c.beneficiary.send(c.amount);
    c.amount = 0;
    return true;
  }
}
{% endhighlight %}

The contract does not provide the full functionality of a crowdfunding
contract, but it contains the basic concepts necessary to understand structs.
Struct types can be used inside mappings and arrays and they can itself
contain mappings and arrays.

It is not possible for a struct to contain a member of its own type,
although the struct itself can be the value type of a mapping member.
This restriction is necessary, as the size of the struct has to be finite.

Note how in all the functions, a struct type is assigned to a local variable
(of the default storage data location).
This does not copy the struct but only stores a reference so that assignments to
members of the local variable actually write to the state.

Of course, you can also directly access the members of the struct without
assigning it to a local variable, as in
`campaigns[campaignID].amount = 0`.

