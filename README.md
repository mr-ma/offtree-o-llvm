# How to run the offtree-o-llvm passes


1. clone the repository
2. cd `offtree-o-llvm` directory
3. run ```docker build .``` (the Dockerfile is placed in the root of the repo)
4. run a container of the built image ```docker run -it [imageid] bash```
5. run offtree obfuscation
```bash obfuscate.sh -o [SUB-FLA-BCF] -a input.bc output.bc```

# Usage samples

```bash obfuscate.sh -o SUB-FLA-BCF -a anagram.bc anagram-SUB-FLA-BCF.bc```

```bash obfuscate.sh -o SUB-FLA -a anagram.bc anagram-SUB-FLA.bc```

```bash obfuscate.sh -o FLA -a anagram.bc anagram-FLA.bc```

***Note*** `anagram.bc` is a sample program shipped with the repository.
