## CircleCI integration

### Docker images

The docker images are build locally on the developer machine:

```sh
cd .circleci/docker/

docker build -t ethereum/solidity-buildpack-deps:ubuntu2204-<revision> -f Dockerfile.ubuntu2204 .
docker push ethereum/solidity-buildpack-deps:ubuntu2204-<revision>
```

The current revisions per docker image are stored in [circle ci pipeline parameters](https://github.com/CircleCI-Public/api-preview-docs/blob/master/docs/pipeline-parameters.md#pipeline-parameters) called `<image-desc>-docker-image-rev` (e.g., `ubuntu-2204-docker-image-rev`). Please update the value assigned to the parameter(s) corresponding to the docker image(s) being updated at the time of the update. Please verify that the value assigned to the parameter matches the revision part of the docker image tag (`<revision>` in the docker build/push snippet shown above). Otherwise, the docker image used by circle ci and the one actually pushed to docker hub will differ.

Once the docker image has been built and pushed to Dockerhub, you can find it at:

    https://hub.docker.com/r/ethereum/solidity-buildpack-deps:ubuntu2204-<revision>

where the image tag reflects the target OS and revision to build Solidity and run its tests on.

### Testing docker images locally

```sh
cd solidity
# Mounts your local solidity directory in docker container for testing
docker run -v `pwd`:/src/solidity -ti ethereum/solidity-buildpack-deps:ubuntu2204-<revision> /bin/bash
cd /src/solidity
<commands_to_test_build_with_new_docker_image>
```
