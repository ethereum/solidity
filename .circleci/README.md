## CircleCI integration

### Docker images

The docker images are build locally on the developer machine:

```!sh
cd .circleci/docker/

docker build -t ethereum/solidity-buildpack-deps:ubuntu1904-<revision> -f Dockerfile.ubuntu1904 .
docker push ethereum/solidity-buildpack-deps:ubuntu1904-<revision>
```

The current revision is stored in a [circle ci pipeline parameter](https://github.com/CircleCI-Public/api-preview-docs/blob/master/docs/pipeline-parameters.md#pipeline-parameters) called `docker-image-rev`. Please update the value assigned to this parameter at the time of a docker image update. Please verify that the value assigned to the parameter matches the revision part of the docker image tag (`<revision>` in the docker build/push snippet shown above). Otherwise, the docker image used by circle ci and the one actually pushed to docker hub will differ.

Once the docker image has been built and pushed to Dockerhub, you can find it at:

    https://hub.docker.com/r/ethereum/solidity-buildpack-deps:ubuntu1904-<revision>

where the image tag reflects the target OS and revision to build Solidity and run its tests on.

### Testing docker images locally

```!sh
cd solidity
# Mounts your local solidity directory in docker container for testing
docker run -v `pwd`:/src/solidity -ti ethereum/solidity-buildpack-deps:ubuntu1904-<revision> /bin/bash
cd /src/solidity
<commands_to_test_build_with_new_docker_image>
```
