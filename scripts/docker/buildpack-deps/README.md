# buildpack-deps docker images

The `buildpack-deps` docker images are used to compile and test solidity within our CI.

## GitHub Workflow

The creation of the images are triggered by a single workflow, defined in `.github/workflows/buildpack-deps.yml`.
For each resulting `buildpack-deps` docker image a strategy is defined in the workflow file - the image variant.
The workflow gets triggered, if any Dockerfile defined in `scripts/docker/buildpack-deps/Dockerfile.*` were changed
within the PR.

### Versioning

The version of the docker images can be defined within the Dockerfile with `LABEL version`. A new docker image
will only be created and pushed, if the new version is incremented by `1` compared with the version of the Dockerfile
located in `develop`.

### Build, Test & Push

Note that the whole workflow - including all defined strategies (image variants) - will be triggered,
even if only a single Dockerfile was changed. The full workflow will only get executed, if the corresponding
Dockerfile was changed. The execution of workflows of unchanged Dockerfiles will not continue and just return success.
See `scripts/ci/docker_upgrade.sh`.

If the version check was successful, the docker image will be built using the Dockerfile located in
`scripts/docker/buildpack-deps/Dockerfile.*`.

The resulting docker image will be tested by executing the corresponding `scripts/ci/buildpack-deps_test_*` scripts.
Some of these scripts are symlinked to `scripts/ci/build.sh`, except the following two:
 * `buildpack-deps-ubuntu.clang.ossfuzz` => `scripts/ci/build_ossfuzz.sh`
 * `buildpack-deps_test_emscripten.sh` => `scripts/ci/build_emscripten.sh`

These scripts `scripts/ci/build.sh` and `scripts/ci/build_ossfuzz.sh` are also used by CircleCI, see `.circleci/config.yml`.

If the tests passed successfully, the docker image will get tagged by the version defined within the corresponding `Dockerfile`.
Finally, a comment will be added to the PR that contains the full repository, version and repository digest
of the freshly created docker image.

The files `.circleci/config.yml` and `scripts/build_emscripten.sh` need to be updated with the new hash posted in the comment.
