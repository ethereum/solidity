(* Generated test file *)
Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.

Require test.libsolidity.semanticTests.externalContracts.deposit_contract.DepositContract.
Require test.libsolidity.semanticTests.externalContracts.deposit_contract.ERC165.
Require test.libsolidity.semanticTests.externalContracts.deposit_contract.IDepositContract.

Definition constructor_code : Code.t :=
  test.libsolidity.semanticTests.externalContracts.deposit_contract.DepositContract.DepositContract.code.

Definition deployed_code : Code.t :=
  test.libsolidity.semanticTests.externalContracts.deposit_contract.DepositContract.DepositContract.deployed.code.

Definition codes : list (U256.t * M.t BlockUnit.t) :=
  test.libsolidity.semanticTests.externalContracts.deposit_contract.DepositContract.codes.

Module Constructor.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := [];
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    let address := environment.(Environment.address) in
    let account := {|
      Account.balance := environment.(Environment.callvalue);
      Account.nonce := 1;
      Account.code := constructor_code.(Code.hex_name);
      Account.codedata := Memory.hex_string_as_bytes "";
      Account.storage := Memory.init;
      Account.immutables := [];
    |} in
    Stdlib.initial_state
      <| State.accounts := [(address, account)] |>
      <| State.codes := codes |>.

  Definition result_state :=
    eval_with_revert 5000 environment constructor_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Goal Test.is_return result state = inl (Memory.u256_as_bytes deployed_code.(Code.hex_name)).
  Proof.
    vm_compute.
    reflexivity.
  Qed.

Definition final_state : State.t :=
  snd (
    eval 5000 environment (update_current_code_for_deploy deployed_code.(Code.hex_name)) state
  ).
End Constructor.

(* // supportsInterface(bytes4): 0x0 -> 0 *)
Module Step1.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "01ffc9a70000000000000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Constructor.final_state.(State.accounts) |>
      <| State.codes := Constructor.final_state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step1.

(* // supportsInterface(bytes4): 0xffffffff00000000000000000000000000000000000000000000000000000000 -> false # defined to be false by ERC-165 # *)
Module Step2.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "01ffc9a7ffffffff00000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step1.state.(State.accounts) |>
      <| State.codes := Step1.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step2.

(* // supportsInterface(bytes4): 0x01ffc9a700000000000000000000000000000000000000000000000000000000 -> true # ERC-165 id # *)
Module Step3.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "01ffc9a701ffc9a700000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step2.state.(State.accounts) |>
      <| State.codes := Step2.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000001".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step3.

(* // supportsInterface(bytes4): 0x8564090700000000000000000000000000000000000000000000000000000000 -> true # the deposit interface id # *)
Module Step4.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "01ffc9a78564090700000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step3.state.(State.accounts) |>
      <| State.codes := Step3.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000001".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step4.

(* // get_deposit_root() -> 0xd70a234731285c6804c2a4f56711ddb8c82c99740f207854891028af34e27e5e
// gas irOptimized: 109178
// gas legacy: 142735
// gas legacyOptimized: 117558 *)
Module Step5.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "c5f2892f";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step4.state.(State.accounts) |>
      <| State.codes := Step4.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "d70a234731285c6804c2a4f56711ddb8c82c99740f207854891028af34e27e5e".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step5.

(* // get_deposit_count() -> 0x20, 8, 0 # TODO: check balance and logs after each deposit # *)
Module Step6.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "621fd130";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step5.state.(State.accounts) |>
      <| State.codes := Step5.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step6.

(* // deposit(bytes,bytes,bytes,bytes32), 32 ether: 0 -> FAILURE # Empty input # *)
Module Step7.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 32000000000000000000;
    Environment.calldata := Memory.hex_string_as_bytes "228951180000000000000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step6.state.(State.accounts) |>
      <| State.codes := Step6.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "".

  Goal Test.extract_output result state Test.Status.Failure = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step7.

(* // get_deposit_root() -> 0xd70a234731285c6804c2a4f56711ddb8c82c99740f207854891028af34e27e5e
// gas irOptimized: 109178
// gas legacy: 142735
// gas legacyOptimized: 117558 *)
Module Step8.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "c5f2892f";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step7.state.(State.accounts) |>
      <| State.codes := Step7.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "d70a234731285c6804c2a4f56711ddb8c82c99740f207854891028af34e27e5e".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step8.

(* // get_deposit_count() -> 0x20, 8, 0 *)
Module Step9.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "621fd130";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step8.state.(State.accounts) |>
      <| State.codes := Step8.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step9.

(* // deposit(bytes,bytes,bytes,bytes32), 1 ether: 0x80, 0xe0, 0x120, 0xaa4a8d0b7d9077248630f1a4701ae9764e42271d7f22b7838778411857fd349e, 0x30, 0x933ad9491b62059dd065b560d256d8957a8c402cc6e8d8ee7290ae11e8f73292, 0x67a8811c397529dac52ae1342ba58c9500000000000000000000000000000000, 0x20, 0x00f50428677c60f997aadeab24aabf7fceaef491c96a52b463ae91f95611cf71, 0x60, 0xa29d01cc8c6296a8150e515b5995390ef841dc18948aa3e79be6d7c1851b4cbb, 0x5d6ff49fa70b9c782399506a22a85193151b9b691245cebafd2063012443c132, 0x4b6c36debaedefb7b2d71b0503ffdc00150aaffd42e63358238ec888901738b8 -> # txhash: 0x7085c586686d666e8bb6e9477a0f0b09565b2060a11f1c4209d3a52295033832 #
// ~ emit DepositEvent(bytes,bytes,bytes,bytes,bytes): 0xa0, 0x0100, 0x0140, 0x0180, 0x0200, 0x30, 0x933ad9491b62059dd065b560d256d8957a8c402cc6e8d8ee7290ae11e8f73292, 0x67a8811c397529dac52ae1342ba58c9500000000000000000000000000000000, 0x20, 0xf50428677c60f997aadeab24aabf7fceaef491c96a52b463ae91f95611cf71, 0x08, 0xca9a3b00000000000000000000000000000000000000000000000000000000, 0x60, 0xa29d01cc8c6296a8150e515b5995390ef841dc18948aa3e79be6d7c1851b4cbb, 0x5d6ff49fa70b9c782399506a22a85193151b9b691245cebafd2063012443c132, 0x4b6c36debaedefb7b2d71b0503ffdc00150aaffd42e63358238ec888901738b8, 0x08, 0x00 *)
Module Step10.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 1000000000000000000;
    Environment.calldata := Memory.hex_string_as_bytes "22895118000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000000000000000e00000000000000000000000000000000000000000000000000000000000000120aa4a8d0b7d9077248630f1a4701ae9764e42271d7f22b7838778411857fd349e0000000000000000000000000000000000000000000000000000000000000030933ad9491b62059dd065b560d256d8957a8c402cc6e8d8ee7290ae11e8f7329267a8811c397529dac52ae1342ba58c9500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000f50428677c60f997aadeab24aabf7fceaef491c96a52b463ae91f95611cf710000000000000000000000000000000000000000000000000000000000000060a29d01cc8c6296a8150e515b5995390ef841dc18948aa3e79be6d7c1851b4cbb5d6ff49fa70b9c782399506a22a85193151b9b691245cebafd2063012443c1324b6c36debaedefb7b2d71b0503ffdc00150aaffd42e63358238ec888901738b8";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step9.state.(State.accounts) |>
      <| State.codes := Step9.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step10.

(* // get_deposit_root() -> 0x2089653123d9c721215120b6db6738ba273bbc5228ac093b1f983badcdc8a438
// gas irOptimized: 109174
// gas legacy: 142744
// gas legacyOptimized: 117570 *)
Module Step11.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "c5f2892f";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step10.state.(State.accounts) |>
      <| State.codes := Step10.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "2089653123d9c721215120b6db6738ba273bbc5228ac093b1f983badcdc8a438".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step11.

(* // get_deposit_count() -> 0x20, 8, 0x0100000000000000000000000000000000000000000000000000000000000000 *)
Module Step12.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "621fd130";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step11.state.(State.accounts) |>
      <| State.codes := Step11.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000080100000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step12.

(* // deposit(bytes,bytes,bytes,bytes32), 32 ether: 0x80, 0xe0, 0x120, 0xdbd986dc85ceb382708cf90a3500f500f0a393c5ece76963ac3ed72eccd2c301, 0x30, 0xb2ce0f79f90e7b3a113ca5783c65756f96c4b4673c2b5c1eb4efc22280259441, 0x06d601211e8866dc5b50dc48a244dd7c00000000000000000000000000000000, 0x20, 0x00344b6c73f71b11c56aba0d01b7d8ad83559f209d0a4101a515f6ad54c89771, 0x60, 0x945caaf82d18e78c033927d51f452ebcd76524497b91d7a11219cb3db6a1d369, 0x7595fc095ce489e46b2ef129591f2f6d079be4faaf345a02c5eb133c072e7c56, 0x0c6c3617eee66b4b878165c502357d49485326bc6b31bc96873f308c8f19c09d -> # txhash: 0x404d8e109822ce448e68f45216c12cb051b784d068fbe98317ab8e50c58304ac #
// ~ emit DepositEvent(bytes,bytes,bytes,bytes,bytes): 0xa0, 0x0100, 0x0140, 0x0180, 0x0200, 0x30, 0xb2ce0f79f90e7b3a113ca5783c65756f96c4b4673c2b5c1eb4efc22280259441, 0x06d601211e8866dc5b50dc48a244dd7c00000000000000000000000000000000, 0x20, 0x344b6c73f71b11c56aba0d01b7d8ad83559f209d0a4101a515f6ad54c89771, 0x08, 0x40597307000000000000000000000000000000000000000000000000000000, 0x60, 0x945caaf82d18e78c033927d51f452ebcd76524497b91d7a11219cb3db6a1d369, 0x7595fc095ce489e46b2ef129591f2f6d079be4faaf345a02c5eb133c072e7c56, 0x0c6c3617eee66b4b878165c502357d49485326bc6b31bc96873f308c8f19c09d, 0x08, 0x0100000000000000000000000000000000000000000000000000000000000000 *)
Module Step13.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 32000000000000000000;
    Environment.calldata := Memory.hex_string_as_bytes "22895118000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000000000000000e00000000000000000000000000000000000000000000000000000000000000120dbd986dc85ceb382708cf90a3500f500f0a393c5ece76963ac3ed72eccd2c3010000000000000000000000000000000000000000000000000000000000000030b2ce0f79f90e7b3a113ca5783c65756f96c4b4673c2b5c1eb4efc2228025944106d601211e8866dc5b50dc48a244dd7c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000344b6c73f71b11c56aba0d01b7d8ad83559f209d0a4101a515f6ad54c897710000000000000000000000000000000000000000000000000000000000000060945caaf82d18e78c033927d51f452ebcd76524497b91d7a11219cb3db6a1d3697595fc095ce489e46b2ef129591f2f6d079be4faaf345a02c5eb133c072e7c560c6c3617eee66b4b878165c502357d49485326bc6b31bc96873f308c8f19c09d";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step12.state.(State.accounts) |>
      <| State.codes := Step12.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step13.

(* // get_deposit_root() -> 0x40255975859377d912c53aa853245ebd939bdd2b33a28e084babdcc1ed8238ee
// gas irOptimized: 109174
// gas legacy: 142744
// gas legacyOptimized: 117570 *)
Module Step14.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "c5f2892f";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step13.state.(State.accounts) |>
      <| State.codes := Step13.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "40255975859377d912c53aa853245ebd939bdd2b33a28e084babdcc1ed8238ee".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step14.

(* // get_deposit_count() -> 0x20, 8, 0x0200000000000000000000000000000000000000000000000000000000000000 *)
Module Step15.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "621fd130";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step14.state.(State.accounts) |>
      <| State.codes := Step14.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000080200000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step15.
