workflow "Build swuniq" {
  on = "push"
  resolves = [
    "Build with gcc",
    "Build with gcc [static]",
    "Build with clang",
  ]
}

action "Build with gcc" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make gcc musl-dev && cd $GITHUB_WORKSPACE && make"]
}

action "Build with gcc [static]" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make gcc musl-dev && cd $GITHUB_WORKSPACE && make static"]
}

action "Build with clang" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make gcc clang musl-dev binutils && cd $GITHUB_WORKSPACE && CC=clang make"]
}
