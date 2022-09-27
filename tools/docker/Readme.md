# How to Build Gramods on Docker

The [`Dockerfile`](Dockerfile) file is a docker configuration for
building on Ubuntu. Below are instructions on how to use this.

## Building the Docker Image

```{bash}
docker build -t gramods-on-Ubuntu .
```

Run (interactively) to copy the resulting files:

```{bash}
docker run -v $(pwd .):/host -it [IMAGE ID]
```

## Other Commands

| Use              | Command                      |
|------------------|------------------------------|
| List containers  | `docker container ls`        |
| Remove container | `docker rm [CONTAINER ID]`   |
| Review logs      | `docker logs [CONTAINER ID]` |
| List images      | `docker image ls`            |
| Remove image     | `docker rmi [IMAGE ID]`      |
