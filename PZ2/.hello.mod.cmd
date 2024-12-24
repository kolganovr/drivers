savedcmd_/home/kolganovr/hello_module/hello.mod := printf '%s\n'   hello.o | awk '!x[$$0]++ { print("/home/kolganovr/hello_module/"$$0) }' > /home/kolganovr/hello_module/hello.mod
