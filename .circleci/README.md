## CircleCI integration

### Docker images

The docker images are build locally on the developer machine:

```!sh
cd .circleci/docker/

docker build -t ethereum/solidity-buildpack-deps:ubuntu1904-<revision> -f Dockerfile.ubuntu1904 .
docker push ethereum/solidity-buildpack-deps:ubuntu1904-<revision>
```

The current revision is `1`.

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
